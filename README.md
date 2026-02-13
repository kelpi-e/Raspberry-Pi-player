Web Interface (Django Playlists Manager)
========================================

Эта часть проекта реализует веб‑интерфейс для работы с плейлистами и треками, которые затем используются C++‑плеером.  
По сути это небольшой Django‑проект, который:

- Хранит плейлисты и треки в JSON‑файлах.  
- Умеет создавать/редактировать/удалять плейлисты.  
- Умеет добавлять треки по ссылкам (Spotify, YouTube) или через загрузку MP3.  
- Извлекает метаданные (название, артист, обложка и т.п.) через внешний `MetadataExtractor`.  


Структура проекта
-----------------

Корень веб‑части: `web-interface/`

- `main.py`  
  - Входная точка для запуска Django‑сервера (обёртка над стандартной `manage.py runserver`).  
  - Настраивает окружение и запускает сервер.

- `player_site/`  
  - Django‑проект (settings, urls, wsgi):
    - `player_site/settings.py`  
      - Настраивает:
        - Пути `BASE_DIR`, `PROJECT_ROOT`.  
        - Загрузку `config.json` из корня репозитория (для внешних сервисов).  
        - Каталоги данных:
          - `DATA_DIR = PROJECT_ROOT / 'data'`  
          - `PLAYLISTS_DIR = DATA_DIR / 'playlists'`  
          - `TRACKS_MEDIA_DIR = DATA_DIR / 'mp3'`  
          - `COVERS_DIR` — каталог с обложками (берётся из `config.json` или по умолчанию `downloaded_covers`).  
        - Статические и медиа‑файлы.  
      - Убеждается, что каталоги `PLAYLISTS_DIR` и `TRACKS_MEDIA_DIR` созданы.
    - `player_site/urls.py`  
      - Маршруты для приложения `playlists` и вспомогательных обработчиков (например, сервинг обложек).
    - `player_site/wsgi.py`  
      - Точка входа WSGI для деплоя (если потребуется).

- `player_site/playlists/`  
  - Django‑приложение, которое отвечает за управление плейлистами:

  - `services.py`  
    - Функции работы с плейлистами и треками на уровне данных:
      - `list_playlists()`  
        - Сканирует `settings.PLAYLISTS_DIR` (`data/playlists/`) на наличие `*.json`.  
        - Для каждого файла читает `name` и количество треков, формируя список словарей `{id, name, track_count}`.  
      - `load_playlist(playlist_id)`  
        - Загружает один плейлист по идентификатору (имя файла без `.json`).  
        - Возвращает `{id, name, tracks}` c нормализованными треками.  
      - `save_playlist(playlist_id, name, tracks)` / `create_playlist(name)` / `delete_playlist(playlist_id)`  
        - Сохранение, создание и удаление плейлистов.  
      - `detect_media_type(source)`  
        - Определяет тип источника (Spotify, YouTube, mp3) по URL/пути.  
      - `extract_metadata(source, media_type, uploaded_file=None)`  
        - Использует `MetadataExtractor` из корня репозитория, чтобы получить данные о треке.  
        - Для MP3 может сохранять загруженный файл в `TRACKS_MEDIA_DIR`.  
        - Возвращает словарь трека (`type, name, artist, duration, path, cover`).
      - `add_track_to_playlist(...)` / `update_playlist_tracks(...)`  
        - Обновление содержимого плейлиста.

  - `views.py`  
    - HTTP‑обработчики:
      - `index(request)`  
        - Показывает список всех плейлистов (страница `templates/playlists/index.html`).  
        - Использует `services.list_playlists()`.  
      - `playlist_edit(request, playlist_id)`  
        - Отображает один плейлист и его треки, форму для добавления и сохранения.  
        - Для каждого трека вычисляет `cover_url` (через `_cover_url`), чтобы подставить правильный путь к обложке.  
      - `playlist_create(request)` (POST)  
        - Создаёт новый плейлист и переходит на его редактирование.  
      - `playlist_save(request, playlist_id)` (POST)  
        - Сохраняет имя плейлиста и полный список треков (можно передать JSON через тело запроса).  
      - `playlist_add_track(request, playlist_id)` (POST)  
        - Добавляет трек по URL или через загрузку MP3, используя `services.extract_metadata`.  
      - `playlist_remove_track(request, playlist_id)` (POST)  
        - Удаляет трек по индексу.  
      - `playlist_delete(request, playlist_id)` (POST)  
        - Удаляет весь плейлист.  
      - `serve_cover(request, path)`  
        - Отдаёт файл обложки, путь к которому хранится как относительный к `PROJECT_ROOT`.  
      - `api_extract_metadata(request)` (POST)  
        - API‑эндпоинт для клиентской части: по URL или MP3 возвращает JSON с метаданными трека.

- Дополнительные файлы в корне веб‑части:
  - `MediaTypes.py` — перечисление или набор констант для типов источников (`SPOTIFY`, `YOUTUBE`, `MP3`).  
  - `MetadataExtractor.py` — обёртка над внешними библиотеками/АПИ, которая фактически достаёт метаданные и обложки.  
  - `data/playlists/` — каталог с JSON‑плейлистами (например, `test.json`).  
  - `cookies.txt` — файл с куками для YouTube, если требуется (читается из конфига).


Установка и запуск
------------------

1. Установить Python и зависимости
   - Рекомендуется использовать виртуальное окружение (`venv` или `conda`).
   - Установить Django и прочие зависимости (примерный список, уточните по фактическому `requirements.txt`, если он есть):
     ```bash
     pip install django
     pip install requests
     # и другие библиотеки, необходимые MetadataExtractor и интеграциям с Spotify/YouTube
     ```

2. Создать `config.json` в корне репозитория (рядом с `web-interface` и `main`)  
   Пример минимального содержимого:
   ```json
   {
     "spotify": {
       "client_id": "YOUR_SPOTIFY_CLIENT_ID",
       "client_secret": "YOUR_SPOTIFY_CLIENT_SECRET"
     },
     "youtube": {
       "cookies_file": "cookies.txt"
     },
     "general": {
       "covers_dir": "downloaded_covers",
       "default_cover_path": "/../res/ui/icons/default.svg"
     }
   }
   ```

3. Структура каталогов данных
   - При первом запуске Django сам создаст:
     - `data/playlists/` — для JSON‑плейлистов.  
     - `data/mp3/` — для загруженных MP3.  
   - Обложки будут сохраняться в каталог, указанный в `covers_dir` (по умолчанию `downloaded_covers` в корне репозитория).

4. Запуск сервера разработки
   - Из каталога `web-interface/player_site`:
     ```bash
     python manage.py runserver 0.0.0.0:8000
     ```
   - Или через `main.py` в корне `web-interface` (если он настроен именно как обёртка):
     ```bash
     python main.py
     ```
   - После запуска веб‑интерфейс будет доступен по адресу `http://localhost:8000/` (адрес может отличаться, если вы задали другой порт).


Использование веб‑интерфейса
----------------------------

1. Перейдите на страницу списка плейлистов (обычно `/playlists/` или `/`, в зависимости от настроек `urls.py`).  
2. Создайте новый плейлист, указав имя.  
3. Откройте плейлист для редактирования:
   - Добавляйте треки по URL из Spotify или YouTube.  
   - Либо загрузите MP3‑файл, чтобы он попал в `data/mp3/`.  
4. При каждом изменении Django обновляет соответствующий JSON в `data/playlists/`.  
5. C++‑плеер читает эти JSON‑файлы и отображает:
   - Список плейлистов (названия и количество треков).  
   - Содержимое выбранного плейлиста (названия, исполнители и длительность).


Связь с C++‑частью
------------------

- Путь хранения плейлистов и формат JSON полностью согласованы с C++‑частью:
  - C++‑часть (`PlaylistScene`) ожидает, что плейлисты находятся в `../web-interface/data/playlists/` относительно директории исполняемого файла.  
  - Формат:
    ```json
    {
      "name": "Имя плейлиста",
      "tracks": [
        {
          "type": "youtube|spotify|mp3",
          "name": "Название трека",
          "artist": "Исполнитель",
          "duration": "MM:SS",
          "path": "...",
          "cover": "..."
        }
      ]
    }
    ```
- Если вы меняете формат JSON или расположение каталогов в Django, не забудьте синхронизировать эти изменения в C++‑коде (в первую очередь в `PlaylistScene`).

