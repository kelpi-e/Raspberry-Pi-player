from MediaTypes import MediaType
import os
import json
from pathlib import Path
import mutagen
from mutagen.mp3 import MP3
from mutagen.id3 import ID3, APIC, TIT2, TPE1, TALB
from typing import Dict, Any, Optional, List
import base64
from io import BytesIO
import yt_dlp
import requests
from urllib.parse import urlparse, parse_qs
import hashlib
import re


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
            try:
                from PIL import Image

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

                # Изменяем размер с сохранением пропорций
                if self.cover_resize_size != (0, 0):
                    img.thumbnail(self.cover_resize_size, Image.Resampling.LANCZOS)

                # Сохраняем
                img.save(save_path, 'JPEG', quality=90, optimize=True)

                print(f"[OK] Обложка сохранена {img.size[0]}x{img.size[1]}: {save_path}")

            except ImportError:
                # Если Pillow не установлен, сохраняем как есть
                print("[!] Pillow не установлен. Сохраняю обложку без изменения размера.")
                print("Установите: pip install pillow")
                with open(save_path, 'wb') as f:
                    f.write(response.content)

            except Exception as pillow_error:
                # Если ошибка при обработке изображения, сохраняем как есть
                print(f"[!] Ошибка при обработке изображения: {pillow_error}")
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
            try:
                from PIL import Image

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

                # Изменяем размер с сохранением пропорций
                if self.cover_resize_size != (0, 0):
                    img.thumbnail(self.cover_resize_size, Image.Resampling.LANCZOS)

                # Сохраняем
                img.save(save_path, 'JPEG', quality=90, optimize=True)

                print(f"[OK] Обложка MP3 сохранена {img.size[0]}x{img.size[1]}: {save_path}")

            except ImportError:
                # Если Pillow не установлен, сохраняем как есть
                print("[!] Pillow не установлен. Сохраняю обложку MP3 без изменения размера.")
                print("Установите: pip install pillow")
                with open(save_path, 'wb') as f:
                    f.write(image_bytes)

            except Exception as pillow_error:
                # Если ошибка при обработке изображения, сохраняем как есть
                print(f"[!] Ошибка при обработке изображения MP3: {pillow_error}")
                with open(save_path, 'wb') as f:
                    f.write(image_bytes)

            return save_path

        except Exception as e:
            print(f"[ERR] Ошибка при сохранении обложки MP3 из байтов: {e}")
            return self.default_cover_path

    def _extract_mp3(self, file_path: str) -> Dict[str, Any]:
        """
        Извлекает метаданные из MP3 файла.
        Возвращает 6 полей: type, name, artist, duration, path, cover
        И добавляет error если есть ошибка
        """
        try:
            # Проверяем существование файла
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

            # Загружаем аудиофайл
            audio = MP3(file_path, ID3=ID3)

            # Получаем базовую информацию
            duration_seconds = audio.info.length if hasattr(audio.info, 'length') else 0

            # Конвертируем секунды в формат MM:SS
            minutes = int(duration_seconds // 60)
            seconds = int(duration_seconds % 60)
            duration_formatted = f"{minutes:02d}:{seconds:02d}"

            # Извлекаем теги
            name = None
            artist = None

            # Пытаемся получить название из тегов
            try:
                if 'TIT2' in audio:
                    name = str(audio['TIT2'])
                elif 'TIT1' in audio:
                    name = str(audio['TIT1'])
            except:
                name = None

            # Если название не найдено, используем имя файла без расширения
            if not name:
                name = os.path.splitext(os.path.basename(file_path))[0]

            # Пытаемся получить исполнителя
            try:
                if 'TPE1' in audio:
                    artist = str(audio['TPE1'])  # Lead performer/soloist
                elif 'TPE2' in audio:
                    artist = str(audio['TPE2'])  # Band/orchestra/accompaniment
                elif 'TPE3' in audio:
                    artist = str(audio['TPE3'])  # Conductor
            except:
                artist = None

            # Если исполнитель не найден, используем "Unknown"
            if not artist:
                artist = "Unknown"

            # Ищем обложку в тегах и сохраняем с изменением размера
            cover_path = self.default_cover_path

            try:
                # Проверяем наличие обложек в тегах
                if audio.tags:
                    for tag in audio.tags.values():
                        if isinstance(tag, APIC):
                            cover_data = tag.data
                            cover_path = self._save_cover_from_bytes(cover_data, name, artist)
                            break
            except Exception as e:
                print(f"Ошибка при извлечении обложки MP3: {e}")

            # Возвращаем результат в нужном формате (ровно 6 полей)
            return {
                "type": "mp3",
                "name": str(name),
                "artist": str(artist),
                "duration": duration_formatted,
                "path": file_path,
                "cover": cover_path
            }

        except Exception as e:
            # В случае ошибки возвращаем структуру с минимальной информацией
            return {
                "type": "mp3",
                "name": os.path.basename(file_path) if 'file_path' in locals() else "Unknown",
                "artist": "Unknown",
                "duration": "00:00",
                "path": file_path if 'file_path' in locals() else "",
                "cover": self.default_cover_path,
                "error": str(e)
            }

    def _extract_youtube(self, url: str) -> Dict[str, Any]:
        """
        Извлекает метаданные из YouTube видео.
        Возвращает 6 полей: type, name, artist, duration, path, cover
        И добавляет error если есть ошибка
        """
        try:
            with yt_dlp.YoutubeDL(self.ydl_opts) as ydl:
                info = ydl.extract_info(url, download=False)

                # Проверяем, есть ли ошибка аутентификации
                if not info:
                    raise Exception("Не удалось получить информацию о видео")

                # Получаем длительность и конвертируем в формат MM:SS
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

                # Получаем название
                name = info.get('title', 'Unknown YouTube Video')
                if len(name) > 100:  # Ограничиваем длину названия
                    name = name[:97] + "..."

                # Получаем исполнителя/автора
                artist = info.get('uploader', info.get('channel', 'Unknown Artist'))

                # Получаем обложку
                thumbnail = info.get('thumbnail')
                cover_url = None

                if isinstance(thumbnail, list) and len(thumbnail) > 0:
                    # Берем самую качественную обложку (последнюю в списке)
                    cover_url = thumbnail[-1]
                elif isinstance(thumbnail, str):
                    cover_url = thumbnail

                # Скачиваем и сохраняем обложку
                if cover_url and cover_url.startswith('http'):
                    cover_path = self._save_cover_from_url(cover_url, name, artist)
                else:
                    cover_path = self.default_cover_path

                # Для YouTube "path" - это исходная ссылка
                path = info.get('webpage_url', url)

                # Возвращаем только 6 полей
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

    def _extract_spotify(self, url: str) -> Dict[str, Any]:
        """
        Извлекает метаданные из Spotify трека только через официальный API.
        Возвращает 6 полей: type, name, artist, duration, path, cover
        И добавляет error если есть ошибка
        """
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

            # Извлекаем ID трека из URL
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

            track_id = path_parts[1].split('?')[0]  # Убираем параметры если есть

            # Делаем запрос к Spotify API
            headers = {
                'Authorization': f'Bearer {self.spotify_token}',
                'Accept': 'application/json',
            }

            response = requests.get(
                f'https://api.spotify.com/v1/tracks/{track_id}',
                headers=headers
            )

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

            # Извлекаем данные
            name = track_data.get('name', 'Unknown Track')
            duration_ms = track_data.get('duration_ms', 0)
            duration_seconds = duration_ms // 1000

            # Конвертируем секунды в формат MM:SS
            minutes = int(duration_seconds // 60)
            seconds = int(duration_seconds % 60)
            duration_formatted = f"{minutes:02d}:{seconds:02d}"

            # Извлекаем исполнителей
            artists = track_data.get('artists', [])
            artist_names = [artist.get('name', '') for artist in artists]
            artist = ', '.join(artist_names) if artist_names else 'Unknown Artist'

            # Извлекаем обложку альбома
            album_images = track_data.get('album', {}).get('images', [])
            cover_url = None

            if album_images:
                # Берем изображение среднего размера (300x300) или первое доступное
                medium_image = next((img for img in album_images if img.get('height', 0) == 300), None)
                if medium_image:
                    cover_url = medium_image.get('url')
                elif album_images:
                    cover_url = album_images[0].get('url')

            # Скачиваем и сохраняем обложку
            if cover_url:
                cover_path = self._save_cover_from_url(cover_url, name, artist)
            else:
                cover_path = self.default_cover_path

            # Возвращаем только 6 полей
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

    def extract_metadata(
            self,
            source: str,
            media_type: MediaType
    ) -> Dict[str, Any]:
        if media_type == MediaType.MP3:
            return self._extract_mp3(file_path=source)
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
