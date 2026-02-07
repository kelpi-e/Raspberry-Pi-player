"""
Playlist and track storage as JSON files in data/playlists/.
Uses MetadataExtractor from project root for MP3/Spotify/YouTube.
"""
import json
import re
import shutil
import uuid
from pathlib import Path

from django.conf import settings

# Import from project root (path set in settings)
from MediaTypes import MediaType
from MetadataExtractor import MetadataExtractor


def _slug(name):
    """Safe filename from playlist name."""
    s = re.sub(r'[^\w\s\-]', '', name).strip()
    s = re.sub(r'[-\s]+', '_', s).lower()
    return s[:80] if s else str(uuid.uuid4())[:8]


def _get_extractor():
    """Build MetadataExtractor from Django settings / config."""
    cfg = getattr(settings, 'EXTERNAL_CONFIG', {}) or {}
    spotify = cfg.get('spotify', {})
    youtube = cfg.get('youtube', {})
    general = cfg.get('general', {})
    cookies_path = None
    if youtube.get('cookies_file'):
        cookies_path = str(settings.PROJECT_ROOT / youtube['cookies_file'])
    return MetadataExtractor(
        spotify_client_id=spotify.get('client_id'),
        spotify_client_secret=spotify.get('client_secret'),
        youtube_cookies_file=cookies_path if cookies_path and Path(cookies_path).exists() else None,
        default_cover_path=general.get('default_cover_path', '/../res/ui/icons/default.svg'),
        covers_dir=str(settings.COVERS_DIR),
    )


def _normalize_track(t):
    """Ensure track has type, name, artist, duration, path, cover; path as relative for MP3 if under MEDIA."""
    t = dict(t)
    for key in ('type', 'name', 'artist', 'duration', 'path', 'cover'):
        t.setdefault(key, '' if key != 'type' else 'mp3')
    if t['type'] == 'mp3' and t.get('path'):
        path = Path(t['path'])
        try:
            path = path.resolve()
            media = Path(settings.MEDIA_ROOT).resolve()
            if path.is_relative_to(media):
                t['path'] = str(path.relative_to(media)).replace('\\', '/')
        except Exception:
            pass
    return t


def list_playlists():
    """List all playlists: [{ id, name, track_count }, ...]."""
    result = []
    for f in Path(settings.PLAYLISTS_DIR).glob('*.json'):
        try:
            data = json.loads(f.read_text(encoding='utf-8'))
            name = data.get('name', f.stem)
            tracks = data.get('tracks', [])
            result.append({
                'id': f.stem,
                'name': name,
                'track_count': len(tracks),
            })
        except Exception:
            continue
    return sorted(result, key=lambda x: x['name'].lower())


def load_playlist(playlist_id):
    """Load one playlist by id (filename without .json). Returns { name, tracks } or None."""
    path = Path(settings.PLAYLISTS_DIR) / f'{playlist_id}.json'
    if not path.exists():
        return None
    data = json.loads(path.read_text(encoding='utf-8'))
    tracks = [_normalize_track(t) for t in data.get('tracks', [])]
    return {
        'id': playlist_id,
        'name': data.get('name', playlist_id),
        'tracks': tracks,
    }


def save_playlist(playlist_id, name, tracks):
    """Save playlist. tracks = list of track dicts (type, name, artist, duration, path, cover)."""
    path = Path(settings.PLAYLISTS_DIR) / f'{playlist_id}.json'
    payload = {
        'name': name,
        'tracks': [_normalize_track(t) for t in tracks],
    }
    path.write_text(json.dumps(payload, ensure_ascii=False, indent=2), encoding='utf-8')
    return True


def create_playlist(name):
    """Create new playlist; returns playlist_id."""
    playlist_id = _slug(name)
    path = Path(settings.PLAYLISTS_DIR) / f'{playlist_id}.json'
    # Uniquify if exists
    base = playlist_id
    n = 0
    while path.exists():
        n += 1
        playlist_id = f'{base}_{n}'
        path = Path(settings.PLAYLISTS_DIR) / f'{playlist_id}.json'
    save_playlist(playlist_id, name, [])
    return playlist_id


def delete_playlist(playlist_id):
    """Remove playlist file."""
    path = Path(settings.PLAYLISTS_DIR) / f'{playlist_id}.json'
    if path.exists():
        path.unlink()
        return True
    return False


def detect_media_type(source):
    """Return MediaType from URL or 'mp3' for file path."""
    s = (source or '').strip()
    if not s:
        return None
    s_lower = s.lower()
    if 'spotify.com' in s_lower and '/track/' in s_lower:
        return MediaType.SPOTIFY
    if 'youtube.com' in s_lower or 'youtu.be' in s_lower:
        return MediaType.YOUTUBE
    if s.endswith('.mp3') or (len(s) < 400 and Path(s).suffix.lower() == '.mp3'):
        return MediaType.MP3
    return None


def extract_metadata(source, media_type, uploaded_file=None):
    """
    Extract metadata for one track.
    source: URL (Spotify/YouTube) or file path (MP3). For MP3 upload, pass uploaded_file and source can be path after save.
    uploaded_file: Django UploadedFile for MP3 (optional); if given, we save to TRACKS_MEDIA_DIR and use that path.
    Returns track dict (type, name, artist, duration, path, cover) or dict with 'error'.
    """
    extractor = _get_extractor()
    path_for_mp3 = None

    if media_type == MediaType.MP3 and uploaded_file:
        # Save uploaded MP3
        safe_name = _slug(uploaded_file.name) or str(uuid.uuid4())[:8]
        ext = Path(uploaded_file.name).suffix or '.mp3'
        dest = Path(settings.TRACKS_MEDIA_DIR) / f'{safe_name}{ext}'
        n = 0
        while dest.exists():
            n += 1
            dest = Path(settings.TRACKS_MEDIA_DIR) / f'{safe_name}_{n}{ext}'
        with open(dest, 'wb') as f:
            for chunk in uploaded_file.chunks():
                f.write(chunk)
        path_for_mp3 = str(dest)

    if media_type == MediaType.MP3:
        file_path = path_for_mp3 or source
        if not file_path or not Path(file_path).exists():
            return {'type': 'mp3', 'name': '', 'artist': '', 'duration': '00:00', 'path': source or '', 'cover': '', 'error': 'File not found'}
        meta = extractor.extract_metadata(file_path, MediaType.MP3)
        # Store path relative to MEDIA_ROOT for portability
        try:
            full = Path(meta['path']).resolve()
            media = Path(settings.MEDIA_ROOT).resolve()
            if full.is_relative_to(media):
                meta['path'] = str(full.relative_to(media)).replace('\\', '/')
        except Exception:
            pass
        return meta
    if media_type == MediaType.SPOTIFY:
        meta = extractor.extract_metadata(source, MediaType.SPOTIFY)
        _normalize_cover_to_relative(meta)
        return meta
    if media_type == MediaType.YOUTUBE:
        meta = extractor.extract_metadata(source, MediaType.YOUTUBE)
        _normalize_cover_to_relative(meta)
        return meta
    return {'type': 'unknown', 'name': '', 'artist': '', 'duration': '00:00', 'path': source, 'cover': '', 'error': 'Unsupported source'}


def _normalize_cover_to_relative(meta):
    """Replace absolute cover path with path relative to PROJECT_ROOT."""
    cover = meta.get('cover')
    if not cover or cover.startswith('http'):
        return
    try:
        full = Path(cover).resolve()
        base = Path(settings.PROJECT_ROOT).resolve()
        if full.is_relative_to(base):
            meta['cover'] = str(full.relative_to(base)).replace('\\', '/')
    except (ValueError, OSError):
        pass


def add_track_to_playlist(playlist_id, track):
    """Append one track to playlist and save."""
    pl = load_playlist(playlist_id)
    if not pl:
        return False
    pl['tracks'].append(_normalize_track(track))
    save_playlist(playlist_id, pl['name'], pl['tracks'])
    return True


def update_playlist_tracks(playlist_id, name, tracks):
    """Replace playlist name and full track list."""
    return save_playlist(playlist_id, name, tracks)
