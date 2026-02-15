import os
import json
import hashlib
import re
import requests
import yt_dlp
from io import BytesIO
from pathlib import Path
from typing import Dict, Any, Optional, List
from urllib.parse import urlparse

from MediaTypes import MediaType

try:
    from PIL import Image
    PILLOW_AVAILABLE = True
except ImportError:
    PILLOW_AVAILABLE = False
    Image = None


class MetadataExtractor:
    def __init__(self,
                 spotify_client_id=None,
                 spotify_client_secret=None,
                 youtube_cookies_browser=None,
                 youtube_cookies_file=None,
                 default_cover_path: str = "/../res/ui/icons/default.svg",
                 covers_dir: str = "covers",
                 cover_resize_size: tuple = (500, 500)):
        """
        Args:
            spotify_client_id: Spotify API client ID
            spotify_client_secret: Spotify API client secret
            youtube_cookies_browser: Браузер для cookies (chrome, firefox, etc)
            youtube_cookies_file: Путь к файлу cookies
            default_cover_path: Путь к обложке по умолчанию
            covers_dir: Директория для сохранения обложек
            cover_resize_size: Размер для изменения обложек (ширина, высота)
        """
        self.default_cover_path = default_cover_path
        self.covers_dir = covers_dir
        self.cover_resize_size = cover_resize_size

        # Создаем директорию для обложек
        os.makedirs(covers_dir, exist_ok=True)

        # Настройка yt-dlp для YouTube
        self.ydl_opts = {
            'quiet': True,
            'no_warnings': True,
            'skip_download': True,
            'ignoreerrors': True,
        }

        # Добавляем cookies если указаны
        if youtube_cookies_browser:
            self.ydl_opts['cookiesfrombrowser'] = (youtube_cookies_browser,)
            print(f"Используем cookies из браузера: {youtube_cookies_browser}")
        elif youtube_cookies_file and os.path.exists(youtube_cookies_file):
            self.ydl_opts['cookiefile'] = youtube_cookies_file
            print(f"Используем файл cookies: {youtube_cookies_file}")

        # Настройка Spotify
        self.spotify_client_id = spotify_client_id
        self.spotify_client_secret = spotify_client_secret
        self.spotify_token = None

        # Инициализация Spotify токена при наличии credentials
        if spotify_client_id and spotify_client_secret:
            self._get_spotify_token()

    def _get_spotify_token(self):
        """Получает токен доступа для Spotify API"""
        try:
            auth_url = 'https://accounts.spotify.com/api/token'
            auth_response = requests.post(
                auth_url,
                data={
                    'grant_type': 'client_credentials',
                    'client_id': self.spotify_client_id,
                    'client_secret': self.spotify_client_secret,
                }
            )

            if auth_response.status_code == 200:
                self.spotify_token = auth_response.json().get('access_token')
                print("Spotify токен успешно получен")
                return True
            else:
                print(f"Ошибка получения токена Spotify: {auth_response.status_code}")
                return False
        except Exception as e:
            print(f"Ошибка при получении Spotify токена: {e}")
            return False

    def _sanitize_filename(self, name: str) -> str:
        """Очищает имя файла от недопустимых символов"""
        # Заменяем недопустимые символы
        invalid_chars = '<>:"/\\|?*'
        for char in invalid_chars:
            name = name.replace(char, '_')

        # Убираем лишние пробелы и ограничиваем длину
        name = '_'.join(filter(None, name.split()))
        if len(name) > 100:
            name = name[:97] + '...'

        return name

    def _save_cover_from_url(self, cover_url: str, track_name: str, artist: str) -> str:
        """
        Скачивает, изменяет размер и сохраняет обложку из URL

        Returns:
            Локальный путь к сохраненной обложке или default_cover_path при ошибке
        """
        if not cover_url or not cover_url.startswith('http'):
            return self.default_cover_path

        try:
            # Генерируем имя файла
            safe_name = self._sanitize_filename(track_name)
            safe_artist = self._sanitize_filename(artist)

            # Хэш URL для уникальности
            url_hash = hashlib.md5(cover_url.encode()).hexdigest()[:8]
            filename = f"{safe_artist}_{safe_name}_{url_hash}.jpg"
            save_path = os.path.join(self.covers_dir, filename)

            # Скачиваем обложку
            headers = {
                'User-Agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36'
            }
            response = requests.get(cover_url, headers=headers, timeout=10)
            response.raise_for_status()

            # Пробуем изменить размер если установлен Pillow
            if PILLOW_AVAILABLE:
                try:
                    # Открываем изображение из байтов
                    img = Image.open(BytesIO(response.content))

                    # Преобразуем в RGB если нужно
                    if img.mode in ('RGBA', 'LA', 'P'):
                        # Создаем белый фон для изображений с прозрачностью
                        rgb_img = Image.new('RGB', img.size, (0, 0, 0))
                        if img.mode == 'P':
                            img = img.convert('RGBA')
                        rgb_img.paste(img, mask=img.split()[-1] if img.mode == 'RGBA' else None)
                        img = rgb_img
                    elif img.mode != 'RGB':
                        img = img.convert('RGB')

                    # Приводим к формату 500x500 (квадрат)
                    if self.cover_resize_size != (0, 0):
                        target_size = self.cover_resize_size[0]  # Используем ширину как размер квадрата
                        # Масштабируем так, чтобы меньшая сторона была равна target_size
                        width, height = img.size
                        if width < height:
                            # Вертикальное изображение - масштабируем по ширине
                            new_width = target_size
                            new_height = int(height * (target_size / width))
                        else:
                            # Горизонтальное или квадратное - масштабируем по высоте
                            new_height = target_size
                            new_width = int(width * (target_size / height))

                        # Масштабируем изображение
                        img = img.resize((new_width, new_height), Image.Resampling.LANCZOS)

                        # Обрезаем до квадрата по центру
                        left = (new_width - target_size) // 2
                        top = (new_height - target_size) // 2
                        right = left + target_size
                        bottom = top + target_size
                        img = img.crop((left, top, right, bottom))

                    # Сохраняем
                    img.save(save_path, 'JPEG', quality=90, optimize=True)

                    print(f"[OK] Обложка сохранена {img.size[0]}x{img.size[1]}: {save_path}")

                except Exception as pillow_error:
                    print(f"[!] Ошибка при обработке изображения: {pillow_error}")
                    with open(save_path, 'wb') as f:
                        f.write(response.content)
            else:
                # Если Pillow не установлен, сохраняем как есть
                print("[!] Pillow не установлен. Сохраняю обложку без изменения размера.")
                print("Установите: pip install pillow")
                with open(save_path, 'wb') as f:
                    f.write(response.content)

            return save_path

        except requests.exceptions.RequestException as e:
            print(f"[ERR] Ошибка при скачивании обложки {cover_url}: {e}")
            return self.default_cover_path
        except Exception as e:
            print(f"[ERR] Неизвестная ошибка при сохранении обложки: {e}")
            return self.default_cover_path

    def _save_cover_from_bytes(self, image_bytes: bytes, track_name: str, artist: str) -> str:
        """
        Сохраняет обложку из байтов с изменением размера (для MP3)

        Returns:
            Локальный путь к сохраненной обложке или default_cover_path при ошибке
        """
        try:
            # Генерируем имя файла
            safe_name = self._sanitize_filename(track_name)
            safe_artist = self._sanitize_filename(artist)

            # Хэш от байтов для уникальности
            image_hash = hashlib.md5(image_bytes).hexdigest()[:8]
            filename = f"{safe_artist}_{safe_name}_{image_hash}.jpg"
            save_path = os.path.join(self.covers_dir, filename)

            # Пробуем изменить размер если установлен Pillow
            if PILLOW_AVAILABLE:
                try:
                    # Открываем изображение из байтов
                    img = Image.open(BytesIO(image_bytes))

                    # Преобразуем в RGB если нужно
                    if img.mode in ('RGBA', 'LA', 'P'):
                        # Создаем белый фон для изображений с прозрачностью
                        rgb_img = Image.new('RGB', img.size, (255, 255, 255))
                        if img.mode == 'P':
                            img = img.convert('RGBA')
                        rgb_img.paste(img, mask=img.split()[-1] if img.mode == 'RGBA' else None)
                        img = rgb_img
                    elif img.mode != 'RGB':
                        img = img.convert('RGB')

                    # Приводим к формату 500x500 (квадрат)
                    if self.cover_resize_size != (0, 0):
                        target_size = self.cover_resize_size[0]  # Используем ширину как размер квадрата
                        # Масштабируем так, чтобы меньшая сторона была равна target_size
                        width, height = img.size
                        if width < height:
                            # Вертикальное изображение - масштабируем по ширине
                            new_width = target_size
                            new_height = int(height * (target_size / width))
                        else:
                            # Горизонтальное или квадратное - масштабируем по высоте
                            new_height = target_size
                            new_width = int(width * (target_size / height))

                        # Масштабируем изображение
                        img = img.resize((new_width, new_height), Image.Resampling.LANCZOS)

                        # Обрезаем до квадрата по центру
                        left = (new_width - target_size) // 2
                        top = (new_height - target_size) // 2
                        right = left + target_size
                        bottom = top + target_size
                        img = img.crop((left, top, right, bottom))

                    # Сохраняем
                    img.save(save_path, 'JPEG', quality=90, optimize=True)

                    print(f"[OK] Обложка MP3 сохранена {img.size[0]}x{img.size[1]}: {save_path}")

                except Exception as pillow_error:
                    print(f"[!] Ошибка при обработке изображения MP3: {pillow_error}")
                    with open(save_path, 'wb') as f:
                        f.write(image_bytes)
            else:
                # Если Pillow не установлен, сохраняем как есть
                print("[!] Pillow не установлен. Сохраняю обложку MP3 без изменения размера.")
                print("Установите: pip install pillow")
                with open(save_path, 'wb') as f:
                    f.write(image_bytes)

            return save_path

        except Exception as e:
            print(f"[ERR] Ошибка при сохранении обложки MP3 из байтов: {e}")
            return self.default_cover_path

    # ========== Извлечение метаданных из MP3 ==========
    def _extract_mp3(self, file_path: str) -> Dict[str, Any]:
        """Извлекает метаданные из MP3 файла."""
        try:
            from mutagen.mp3 import MP3
            from mutagen.id3 import ID3, APIC, TIT2, TPE1

            if not os.path.exists(file_path):
                return {
                    "type": "mp3",
                    "name": os.path.basename(file_path),
                    "artist": "Unknown",
                    "duration": "00:00",
                    "path": file_path,
                    "cover": self.default_cover_path,
                    "error": "File not found"
                }

            audio = MP3(file_path, ID3=ID3)
            duration_seconds = audio.info.length if hasattr(audio.info, 'length') else 0
            minutes = int(duration_seconds // 60)
            seconds = int(duration_seconds % 60)
            duration_formatted = f"{minutes:02d}:{seconds:02d}"

            name = None
            artist = None

            if 'TIT2' in audio:
                name = str(audio['TIT2'])
            if not name:
                name = os.path.splitext(os.path.basename(file_path))[0]

            if 'TPE1' in audio:
                artist = str(audio['TPE1'])
            if not artist:
                artist = "Unknown"

            cover_path = self.default_cover_path
            if audio.tags:
                for tag in audio.tags.values():
                    if isinstance(tag, APIC):
                        cover_data = tag.data
                        cover_path = self._save_cover_from_bytes(cover_data, name, artist)
                        break

            return {
                "type": "mp3",
                "name": str(name),
                "artist": str(artist),
                "duration": duration_formatted,
                "path": file_path,
                "cover": cover_path
            }
        except Exception as e:
            return {
                "type": "mp3",
                "name": os.path.basename(file_path) if 'file_path' in locals() else "Unknown",
                "artist": "Unknown",
                "duration": "00:00",
                "path": file_path if 'file_path' in locals() else "",
                "cover": self.default_cover_path,
                "error": str(e)
            }

    # ========== Извлечение метаданных из YouTube ==========
    def _extract_youtube(self, url: str) -> Dict[str, Any]:
        try:
            with yt_dlp.YoutubeDL(self.ydl_opts) as ydl:
                info = ydl.extract_info(url, download=False)
                if not info:
                    raise Exception("Не удалось получить информацию о видео")

                duration_seconds = info.get('duration', 0)
                if duration_seconds:
                    hours = int(duration_seconds // 3600)
                    minutes = int((duration_seconds % 3600) // 60)
                    seconds = int(duration_seconds % 60)
                    if hours > 0:
                        duration_formatted = f"{hours:02d}:{minutes:02d}:{seconds:02d}"
                    else:
                        duration_formatted = f"{minutes:02d}:{seconds:02d}"
                else:
                    duration_formatted = "00:00"

                name = info.get('title', 'Unknown YouTube Video')
                if len(name) > 100:
                    name = name[:97] + "..."

                artist = info.get('uploader', info.get('channel', 'Unknown Artist'))
                thumbnail = info.get('thumbnail')
                cover_url = None

                if isinstance(thumbnail, list) and thumbnail:
                    cover_url = thumbnail[-1]
                elif isinstance(thumbnail, str):
                    cover_url = thumbnail

                if cover_url and cover_url.startswith('http'):
                    cover_path = self._save_cover_from_url(cover_url, name, artist)
                else:
                    cover_path = self.default_cover_path

                path = info.get('webpage_url', url)

                return {
                    "type": "youtube",
                    "name": str(name),
                    "artist": str(artist),
                    "duration": duration_formatted,
                    "path": str(path),
                    "cover": cover_path
                }
        except Exception as e:
            print(f"Ошибка при извлечении YouTube метаданных: {e}")
            return {
                "type": "youtube",
                "name": "Unknown",
                "artist": "Unknown",
                "duration": "00:00",
                "path": url,
                "cover": self.default_cover_path,
                "error": str(e)
            }

    # ========== Извлечение метаданных из Spotify ==========
    def _extract_spotify(self, url: str) -> Dict[str, Any]:
        try:
            if not self.spotify_token:
                if not self._get_spotify_token():
                    return {
                        "type": "spotify",
                        "name": "Unknown",
                        "artist": "Unknown",
                        "duration": "00:00",
                        "path": url,
                        "cover": self.default_cover_path,
                        "error": "Spotify API credentials not configured or failed to get token"
                    }

            parsed_url = urlparse(url)
            path_parts = parsed_url.path.strip('/').split('/')
            if len(path_parts) < 2 or path_parts[0] != 'track':
                return {
                    "type": "spotify",
                    "name": "Unknown",
                    "artist": "Unknown",
                    "duration": "00:00",
                    "path": url,
                    "cover": self.default_cover_path,
                    "error": "Invalid Spotify track URL"
                }

            track_id = path_parts[1].split('?')[0]
            headers = {'Authorization': f'Bearer {self.spotify_token}'}
            response = requests.get(f'https://api.spotify.com/v1/tracks/{track_id}', headers=headers)

            if response.status_code != 200:
                return {
                    "type": "spotify",
                    "name": "Unknown",
                    "artist": "Unknown",
                    "duration": "00:00",
                    "path": url,
                    "cover": self.default_cover_path,
                    "error": f"Spotify API error: {response.status_code}"
                }

            track_data = response.json()
            name = track_data.get('name', 'Unknown Track')
            duration_ms = track_data.get('duration_ms', 0)
            duration_seconds = duration_ms // 1000
            minutes = int(duration_seconds // 60)
            seconds = int(duration_seconds % 60)
            duration_formatted = f"{minutes:02d}:{seconds:02d}"

            artists = track_data.get('artists', [])
            artist_names = [a.get('name', '') for a in artists]
            artist = ', '.join(artist_names) if artist_names else 'Unknown Artist'

            album_images = track_data.get('album', {}).get('images', [])
            cover_url = None
            if album_images:
                medium = next((img for img in album_images if img.get('height') == 300), None)
                cover_url = (medium or album_images[0]).get('url')

            if cover_url:
                cover_path = self._save_cover_from_url(cover_url, name, artist)
            else:
                cover_path = self.default_cover_path

            return {
                "type": "spotify",
                "name": str(name),
                "artist": str(artist),
                "duration": duration_formatted,
                "path": url,
                "cover": cover_path
            }
        except Exception as e:
            return {
                "type": "spotify",
                "name": "Unknown",
                "artist": "Unknown",
                "duration": "00:00",
                "path": url,
                "cover": self.default_cover_path,
                "error": str(e)
            }

    # ========== Основной метод для одного трека ==========
    def extract_metadata(self, source: str, media_type: MediaType) -> Dict[str, Any]:
        if media_type == MediaType.MP3:
            return self._extract_mp3(source)
        elif media_type == MediaType.SPOTIFY:
            return self._extract_spotify(source)
        elif media_type == MediaType.YOUTUBE:
            return self._extract_youtube(source)
        else:
            raise ValueError(f"unsupported media type: {media_type}")

    def save_metadata_to_json(self, metadata: Dict[str, Any], output_file: str):
        """Сохраняет метаданные в JSON файл с кириллицей"""
        with open(output_file, 'w', encoding='utf-8') as f:
            json.dump(metadata, f, ensure_ascii=False, indent=2)

    # ========== Методы для плейлистов (новые) ==========
    def extract_youtube_playlist(self, url: str) -> List[Dict[str, Any]]:
        """
        Извлекает метаданные для всех видео в YouTube-плейлисте.
        Возвращает список треков в формате, аналогичном extract_metadata.
        """
        tracks = []
        ydl_opts_flat = self.ydl_opts.copy()
        ydl_opts_flat['extract_flat'] = True
        ydl_opts_flat['quiet'] = True

        try:
            with yt_dlp.YoutubeDL(ydl_opts_flat) as ydl:
                info = ydl.extract_info(url, download=False)
                if not info:
                    raise Exception("Не удалось получить информацию о плейлисте")

                entries = info.get('entries', [])
                for entry in entries:
                    if not entry:
                        continue
                    video_url = entry.get('url') or entry.get('webpage_url')
                    if not video_url and entry.get('id'):
                        video_url = f"https://www.youtube.com/watch?v={entry['id']}"
                    if not video_url:
                        continue

                    duration = entry.get('duration', 0)
                    if duration:
                        minutes = int(duration // 60)
                        seconds = int(duration % 60)
                        duration_formatted = f"{minutes:02d}:{seconds:02d}"
                    else:
                        duration_formatted = "00:00"

                    title = entry.get('title', 'Unknown')
                    if len(title) > 100:
                        title = title[:97] + "..."

                    artist = entry.get('uploader') or entry.get('channel') or 'Unknown Artist'

                    thumbnails = entry.get('thumbnails', [])
                    cover_url = None
                    if thumbnails:
                        cover_url = thumbnails[-1].get('url') if isinstance(thumbnails[-1], dict) else thumbnails[-1]
                    if not cover_url:
                        cover_url = entry.get('thumbnail')

                    cover_path = self.default_cover_path
                    if cover_url and cover_url.startswith('http'):
                        cover_path = self._save_cover_from_url(cover_url, title, artist)

                    track = {
                        'type': 'youtube',
                        'name': title,
                        'artist': artist,
                        'duration': duration_formatted,
                        'path': video_url,
                        'cover': cover_path
                    }
                    tracks.append(track)
        except Exception as e:
            print(f"Ошибка при извлечении YouTube-плейлиста: {e}")

        return tracks

    def extract_spotify_playlist(self, url: str) -> List[Dict[str, Any]]:
        """
        Извлекает метаданные для всех треков в Spotify-плейлисте.
        Возвращает список треков в формате, аналогичном extract_metadata.
        """
        tracks = []

        if not self.spotify_token:
            if not self._get_spotify_token():
                print("Не удалось получить токен Spotify API")
                return tracks

        parsed_url = urlparse(url)
        path_parts = parsed_url.path.strip('/').split('/')
        if len(path_parts) < 2 or path_parts[0] != 'playlist':
            print("Некорректный URL плейлиста Spotify")
            return tracks

        playlist_id = path_parts[1].split('?')[0]

        headers = {'Authorization': f'Bearer {self.spotify_token}'}
        next_url = f'https://api.spotify.com/v1/playlists/{playlist_id}/tracks?limit=50'

        while next_url:
            try:
                response = requests.get(next_url, headers=headers)
                if response.status_code != 200:
                    print(f"Ошибка Spotify API: {response.status_code}")
                    break

                data = response.json()
                items = data.get('items', [])

                for item in items:
                    track_data = item.get('track')
                    if not track_data:
                        continue

                    name = track_data.get('name', 'Unknown Track')
                    duration_ms = track_data.get('duration_ms', 0)
                    duration_seconds = duration_ms // 1000
                    minutes = duration_seconds // 60
                    seconds = duration_seconds % 60
                    duration_formatted = f"{minutes:02d}:{seconds:02d}"

                    artists = track_data.get('artists', [])
                    artist_names = [a.get('name', '') for a in artists]
                    artist = ', '.join(artist_names) if artist_names else 'Unknown Artist'

                    album_images = track_data.get('album', {}).get('images', [])
                    cover_url = None
                    if album_images:
                        medium = next((img for img in album_images if img.get('height') == 300), None)
                        cover_url = (medium or album_images[0]).get('url')

                    cover_path = self.default_cover_path
                    if cover_url:
                        cover_path = self._save_cover_from_url(cover_url, name, artist)

                    track = {
                        'type': 'spotify',
                        'name': name,
                        'artist': artist,
                        'duration': duration_formatted,
                        'path': track_data.get('external_urls', {}).get('spotify', ''),
                        'cover': cover_path
                    }
                    tracks.append(track)

                next_url = data.get('next')
            except Exception as e:
                print(f"Ошибка при обработке Spotify-плейлиста: {e}")
                break

        return tracks

    def extract_playlist(self, url: str, media_type: MediaType) -> List[Dict[str, Any]]:
        """
        Универсальный метод для извлечения плейлиста по его типу.
        """
        if media_type == MediaType.YOUTUBE_PLAYLIST:
            return self.extract_youtube_playlist(url)
        elif media_type == MediaType.SPOTIFY_PLAYLIST:
            return self.extract_spotify_playlist(url)
        else:
            raise ValueError(f"Неподдерживаемый тип плейлиста: {media_type}")