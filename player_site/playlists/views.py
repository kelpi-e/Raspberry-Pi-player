import json
import os
from pathlib import Path as PathLib
from django.shortcuts import render, redirect
from django.views.decorators.http import require_http_methods
from django.views.decorators.csrf import ensure_csrf_cookie
from django.http import JsonResponse
from django.conf import settings
from django.http import FileResponse, Http404
from MediaTypes import MediaType

from . import services


def _abs_cover_path(cover):
    """Return URL or path for cover image."""
    if not cover:
        return None
    if cover.startswith('http'):
        return cover
    path = getattr(settings, 'COVERS_DIR', None) or (settings.PROJECT_ROOT / 'downloaded_covers')
    full = path / cover if not (path / cover).is_absolute() else path
    if full.exists():
        return None  # will serve via /media/ or static; we use relative from project
    # Covers saved in COVERS_DIR - need a view to serve them or put under MEDIA
    return str(settings.PROJECT_ROOT / cover).replace('\\', '/') if cover else None


@ensure_csrf_cookie
def index(request):
    """List all playlists."""
    playlists = services.list_playlists()
    return render(request, 'playlists/index.html', {'playlists': playlists})


def _cover_url(cover):
    """Build URL for cover image (external URL or /covers/...)."""
    if not cover:
        return None
    if cover.startswith('http://') or cover.startswith('https://'):
        return cover
    path = cover.replace('\\', '/')
    # If absolute path (e.g. C:/.../downloaded_covers/file.jpg), take part under PROJECT_ROOT
    base = PathLib(settings.PROJECT_ROOT).resolve()
    base_str = str(base).replace('\\', '/')
    path_norm = path.replace('\\', '/')
    if path_norm.upper().startswith(('C:', 'D:', 'E:')) and base_str in path_norm:
        idx = path_norm.find(base_str) + len(base_str)
        path = path_norm[idx:].lstrip('/')
    else:
        path = path_norm.lstrip('/')
    if not path:
        return None
    from django.urls import reverse
    return reverse('playlists:serve_cover', kwargs={'path': path})


@ensure_csrf_cookie
def playlist_edit(request, playlist_id):
    """View/edit one playlist: show name, tracks, form to add and save."""
    pl = services.load_playlist(playlist_id)
    if not pl:
        return redirect('playlists:index')
    for t in pl['tracks']:
        t['cover_url'] = _cover_url(t.get('cover'))
    add_error = request.GET.get('add_error', '')
    return render(request, 'playlists/playlist_edit.html', {'playlist': pl, 'add_error': add_error})


@require_http_methods(['POST'])
def playlist_create(request):
    """Create new playlist by name, redirect to edit."""
    name = (request.POST.get('name') or '').strip() or 'New Playlist'
    playlist_id = services.create_playlist(name)
    return redirect('playlists:playlist_edit', playlist_id=playlist_id)


@require_http_methods(['POST'])
def playlist_save(request, playlist_id):
    """Save playlist name and tracks (JSON in POST)."""
    pl = services.load_playlist(playlist_id)
    if not pl:
        return redirect('playlists:index')
    name = (request.POST.get('name') or request.body and json.loads(request.body).get('name') or pl['name'] or '').strip()
    tracks_raw = request.POST.get('tracks')
    if not tracks_raw and request.content_type and 'json' in request.content_type and request.body:
        data = json.loads(request.body)
        name = (data.get('name') or name or '').strip()
        tracks_raw = data.get('tracks')
    if tracks_raw is not None:
        try:
            tracks = json.loads(tracks_raw) if isinstance(tracks_raw, str) else tracks_raw
        except json.JSONDecodeError:
            tracks = pl['tracks']
    else:
        tracks = pl['tracks']
    services.save_playlist(playlist_id, name or 'Unnamed', tracks)
    if request.headers.get('X-Requested-With') == 'XMLHttpRequest' or request.GET.get('json'):
        return JsonResponse({'ok': True})
    return redirect('playlists:playlist_edit', playlist_id=playlist_id)


@require_http_methods(['POST'])
def playlist_add_track(request, playlist_id):
    """Add one track: either url= (Spotify/YouTube) or mp3 file upload. Redirect back to edit."""
    pl = services.load_playlist(playlist_id)
    if not pl:
        return redirect('playlists:index')

    url = (request.POST.get('url') or '').strip()
    mp3_file = request.FILES.get('mp3')

    if url:
        media_type = services.detect_media_type(url)
        if not media_type:
            if request.headers.get('X-Requested-With') == 'XMLHttpRequest':
                return JsonResponse({'error': 'Unsupported URL'}, status=400)
            return redirect('playlists:playlist_edit', playlist_id=playlist_id)
        track = services.extract_metadata(url, media_type, uploaded_file=None)
    elif mp3_file and mp3_file.name.lower().endswith('.mp3'):
        track = services.extract_metadata(None, MediaType.MP3, uploaded_file=mp3_file)
    else:
        if request.headers.get('X-Requested-With') == 'XMLHttpRequest':
            return JsonResponse({'error': 'Provide URL or MP3 file'}, status=400)
        return redirect('playlists:playlist_edit', playlist_id=playlist_id)

    if track.get('error'):
        if request.headers.get('X-Requested-With') == 'XMLHttpRequest':
            return JsonResponse({'error': track['error'], 'track': track}, status=400)
        from django.urls import reverse
        from urllib.parse import urlencode
        url = reverse('playlists:playlist_edit', kwargs={'playlist_id': playlist_id})
        url += '?' + urlencode({'add_error': track['error']})
        return redirect(url)

    services.add_track_to_playlist(playlist_id, track)

    if request.headers.get('X-Requested-With') == 'XMLHttpRequest':
        pl2 = services.load_playlist(playlist_id)
        return JsonResponse({'ok': True, 'tracks': pl2['tracks']})
    return redirect('playlists:playlist_edit', playlist_id=playlist_id)


@require_http_methods(['POST'])
def playlist_remove_track(request, playlist_id):
    """Remove track by index (0-based)."""
    pl = services.load_playlist(playlist_id)
    if not pl:
        if request.headers.get('X-Requested-With') == 'XMLHttpRequest':
            return JsonResponse({'error': 'Not found'}, status=404)
        return redirect('playlists:index')
    
    # Получаем индекс из POST или JSON body
    index = -1
    try:
        if request.POST.get('index'):
            index = int(request.POST.get('index'))
        elif request.content_type and 'json' in request.content_type and request.body:
            data = json.loads(request.body)
            index = int(data.get('index', -1))
    except (TypeError, ValueError, json.JSONDecodeError) as e:
        print(f"Error parsing index: {e}")
        index = -1
    
    # Удаляем трек если индекс валидный
    if 0 <= index < len(pl['tracks']):
        pl['tracks'].pop(index)
        services.save_playlist(playlist_id, pl['name'], pl['tracks'])
    
    if request.headers.get('X-Requested-With') == 'XMLHttpRequest':
        pl2 = services.load_playlist(playlist_id)
        return JsonResponse({'ok': True, 'tracks': pl2['tracks']})
    return redirect('playlists:playlist_edit', playlist_id=playlist_id)


@require_http_methods(['POST'])
def playlist_delete(request, playlist_id):
    """Delete playlist, redirect to index."""
    services.delete_playlist(playlist_id)
    return redirect('playlists:index')


def serve_cover(request, path):
    """Serve cover image; path is relative to PROJECT_ROOT (e.g. downloaded_covers/file.jpg)."""
    base = PathLib(settings.PROJECT_ROOT).resolve()
    # Normalize path: no parent traversal
    path = path.replace('\\', '/').lstrip('/')
    if '..' in path:
        raise Http404()
    full = (base / path).resolve()
    try:
        if not full.exists() or not full.is_file():
            raise Http404()
        if os.path.commonpath([str(base), str(full)]) != str(base):
            raise Http404()
    except (ValueError, OSError):
        raise Http404()
    return FileResponse(open(full, 'rb'), content_type='image/jpeg')


def api_extract_metadata(request):
    """POST url= or multipart mp3 file -> return track JSON (for client-side add/edit)."""
    url = (request.POST.get('url') or '').strip()
    mp3_file = request.FILES.get('mp3')
    if url:
        media_type = services.detect_media_type(url)
        if not media_type:
            return JsonResponse({'error': 'Unsupported URL'}, status=400)
        track = services.extract_metadata(url, media_type, uploaded_file=None)
    elif mp3_file and mp3_file.name.lower().endswith('.mp3'):
        track = services.extract_metadata(None, MediaType.MP3, uploaded_file=mp3_file)
    else:
        return JsonResponse({'error': 'Provide url or mp3 file'}, status=400)
    return JsonResponse(track)
