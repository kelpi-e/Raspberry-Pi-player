import json
import os
from pathlib import Path

# Project root (Playerserver)
BASE_DIR = Path(__file__).resolve().parent.parent
# Parent = Playerserver repo root (where MediaTypes.py, config.json live)
PROJECT_ROOT = BASE_DIR.parent

# Add project root to path for MediaTypes, MetadataExtractor
import sys
if str(PROJECT_ROOT) not in sys.path:
    sys.path.insert(0, str(PROJECT_ROOT))

# Load config from project root
CONFIG_PATH = PROJECT_ROOT / 'config.json'
if CONFIG_PATH.exists():
    with open(CONFIG_PATH, 'r', encoding='utf-8') as f:
        EXTERNAL_CONFIG = json.load(f)
else:
    EXTERNAL_CONFIG = {}

SECRET_KEY = os.environ.get('DJANGO_SECRET_KEY', 'dev-secret-change-in-production')
DEBUG = True
ALLOWED_HOSTS = ['*']

INSTALLED_APPS = [
    'django.contrib.contenttypes',
    'django.contrib.staticfiles',
    'playlists',
]

MIDDLEWARE = [
    'django.middleware.security.SecurityMiddleware',
    'django.middleware.common.CommonMiddleware',
    'django.middleware.csrf.CsrfViewMiddleware',
    'django.middleware.clickjacking.XFrameOptionsMiddleware',
]

ROOT_URLCONF = 'player_site.urls'
TEMPLATES = [
    {
        'BACKEND': 'django.template.backends.django.DjangoTemplates',
        'DIRS': [BASE_DIR / 'templates'],
        'APP_DIRS': True,
        'OPTIONS': {
            'context_processors': [],
        },
    },
]
WSGI_APPLICATION = 'player_site.wsgi.application'

DATABASES = {}

# Playlists and media stored in project data folder
DATA_DIR = PROJECT_ROOT / 'data'
PLAYLISTS_DIR = DATA_DIR / 'playlists'
TRACKS_MEDIA_DIR = DATA_DIR / 'mp3'
COVERS_DIR = PROJECT_ROOT / EXTERNAL_CONFIG.get('general', {}).get('covers_dir', 'downloaded_covers')

# Ensure dirs exist
PLAYLISTS_DIR.mkdir(parents=True, exist_ok=True)
TRACKS_MEDIA_DIR.mkdir(parents=True, exist_ok=True)

STATIC_URL = 'static/'
STATICFILES_DIRS = [BASE_DIR / 'static'] if (BASE_DIR / 'static').exists() else []
STATIC_ROOT = BASE_DIR / 'staticfiles'

MEDIA_URL = '/media/'
MEDIA_ROOT = DATA_DIR

DEFAULT_AUTO_FIELD = 'django.db.models.BigAutoField'

# File upload
FILE_UPLOAD_MAX_MEMORY_SIZE = 50 * 1024 * 1024  # 50 MB
