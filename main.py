import os
import json
from MediaTypes import MediaType
from MetadataExtractor import MetadataExtractor


def load_config(config_file="config.json"):
    """Простая функция загрузки конфигурации"""
    if not os.path.exists(config_file):
        print(f"Ошибка: файл {config_file} не найден")
        print("Создайте файл config.json с настройками")
        return None

    try:
        with open(config_file, 'r', encoding='utf-8') as f:
            return json.load(f)
    except Exception as e:
        print(f"Ошибка при чтении {config_file}: {e}")
        return None



if __name__ == "__main__":
    config = load_config()
    if not config:
        exit(1)

    # Извлекаем настройки
    spotify_cfg = config.get("spotify", {})
    youtube_cfg = config.get("youtube", {})
    general_cfg = config.get("general", {})

    # Создаем экстракторы
    extractor = MetadataExtractor(
        default_cover_path=general_cfg.get("default_cover_path", "/../res/ui/icons/default.svg"),
        covers_dir=general_cfg.get("covers_dir", "downloaded_covers")
    )

    extractor_cookie = MetadataExtractor(
        youtube_cookies_file=youtube_cfg.get("cookies_file"),
        default_cover_path=general_cfg.get("default_cover_path", "/../res/ui/icons/default.svg"),
        covers_dir=general_cfg.get("covers_dir", "downloaded_covers")
    )

    extractor_spapi = MetadataExtractor(
        spotify_client_id=spotify_cfg.get("client_id"),
        spotify_client_secret=spotify_cfg.get("client_secret"),
        default_cover_path=general_cfg.get("default_cover_path", "/../res/ui/icons/default.svg"),
        covers_dir=general_cfg.get("covers_dir", "downloaded_covers")
    )

    # Далее ваш существующий код тестирования...
    print("Тестирование MetadataExtractor...")

    # Тест 1: YouTube
    print("\n1. Тест YouTube:")
    youtube_url = "https://www.youtube.com/watch?v=dQw4w9WgXcQ"
    try:
        youtube_meta = extractor_cookie.extract_metadata(youtube_url, MediaType.YOUTUBE)
        print(json.dumps(youtube_meta, ensure_ascii=False, indent=2))
    except Exception as e:
        print(f"Ошибка YouTube: {e}")

    # Тест 2: Spotify
    print("\n2. Тест Spotify:")
    if extractor_spapi.spotify_client_id:
        spotify_url = "https://open.spotify.com/track/4QC7hxWVrvfYh93eGIgrzL"
        try:
            spotify_meta = extractor_spapi.extract_metadata(spotify_url, MediaType.SPOTIFY)
            print(json.dumps(spotify_meta, ensure_ascii=False, indent=2))
        except Exception as e:
            print(f"Ошибка Spotify: {e}")
    else:
        print("Spotify API не настроен в config.json")

    # Тест 3: MP3
    print("\n3. Тест MP3:")
    mp3_path = "C:\\Users\\al_ru\\Downloads\\СЕРЕГА ПИРАТ - И я кричу, остановите катку.mp3"
    if os.path.exists(mp3_path):
        try:
            mp3_meta = extractor.extract_metadata(mp3_path, MediaType.MP3)
            print(json.dumps(mp3_meta, ensure_ascii=False, indent=2))
        except Exception as e:
            print(f"Ошибка MP3: {e}")
    else:
        print(f"MP3 файл не найден: {mp3_path}")

