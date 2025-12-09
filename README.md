# Raspberry Pi Audio Player

Медиаплеер для Raspberry Pi с поддержкой воспроизведения MP3 файлов и потокового аудио с YouTube. Приложение разработано с использованием Qt6 и предназначено для работы на Raspberry Pi с поворотом интерфейса на 90 градусов для удобного отображения на вертикальном дисплее.

## Возможности

- Воспроизведение локальных MP3 файлов
- Потоковое воспроизведение аудио с YouTube
- Графический интерфейс с поддержкой поворота экрана
- Отображение метаданных треков (название, исполнитель)
- Управление воспроизведением (пауза, перемотка)
- Прогресс-бар с отображением времени

## Технологии

- **Qt6** (Core, Widgets, Multimedia) - графический интерфейс и медиа-функции
- **TagLib** - чтение метаданных из MP3 файлов
- **yt-dlp** - извлечение аудио-потоков с YouTube
- **C++20** - язык программирования

## Требования

### Системные зависимости

- CMake 3.16 или выше
- Компилятор C++ с поддержкой C++20 (GCC 10+, Clang 12+)
- vcpkg для управления зависимостями
- Python 3 (для работы с yt-dlp)
- yt-dlp (установка через pip: `pip install yt-dlp`)

### Платформы

- Raspberry Pi OS (Linux)
- Windows (для разработки)
- Linux (для разработки)

## Установка и сборка

### 1. Установка vcpkg

Если у вас еще не установлен vcpkg:

```bash
# Клонируем репозиторий vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# На Linux/macOS
./bootstrap-vcpkg.sh

# На Windows
.\bootstrap-vcpkg.bat
```

### 2. Установка зависимостей через vcpkg

```bash
# Установка зависимостей из manifest файла
vcpkg install --triplet x64-linux  # Для Linux/Raspberry Pi
# или
vcpkg install --triplet x64-windows  # Для Windows
```

### 3. Клонирование репозитория

```bash
git clone <your-repo-url>
cd Raspberry-Pi-player
```

### 4. Сборка проекта

#### На Linux/Raspberry Pi:

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake
cmake --build .
```

#### На Windows:

```powershell
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>\scripts\buildsystems\vcpkg.cmake
cmake --build .
```

### 5. Установка yt-dlp

Для работы с YouTube необходимо установить yt-dlp:

```bash
pip install yt-dlp
```

Или через пакетный менеджер:

```bash
# На Raspberry Pi OS
sudo apt-get install yt-dlp

# Или через pip
pip3 install yt-dlp
```

## Запуск приложения

### Подготовка

1. Убедитесь, что в директории `res/music/` есть MP3 файлы для тестирования
2. Проверьте, что yt-dlp установлен и доступен в PATH

### Запуск

```bash
# Из директории build
./Raspberry-Pi-player  # Linux/Raspberry Pi
# или
.\Raspberry-Pi-player.exe  # Windows
```

### Использование

- Приложение автоматически запускается с тестовым YouTube URL
- Для воспроизведения локальных MP3 файлов измените код в `main.cpp` или добавьте функционал выбора файла
- Интерфейс повернут на 90 градусов для вертикального дисплея Raspberry Pi

## Структура проекта

```
Raspberry-Pi-player/
├── CMakeLists.txt          # Конфигурация сборки CMake
├── vcpkg.json              # Манифест зависимостей vcpkg
├── README.md               # Документация проекта
├── res/
│   ├── main.cpp           # Точка входа приложения
│   ├── pch.h              # Precompiled headers
│   ├── audio/             # Модуль воспроизведения аудио
│   │   ├── PlayerAudio.h/cpp
│   │   ├── mediafactory.h
│   │   ├── mediastategy.h
│   │   ├── mp3strategy.h
│   │   └── youtubestrategy.h
│   ├── ui/                # Пользовательский интерфейс
│   │   ├── PlayerWindow.h/cpp
│   │   └── PlayerWindow.ui
│   ├── music/             # Директория для MP3 файлов
│   └── python/            # Python скрипты (если требуется)
└── build/                 # Директория сборки (создается)
```

## Архитектура

Проект использует паттерн Strategy для поддержки различных типов медиа:

- **Mp3Strategy** - обработка локальных MP3 файлов
- **YoutubeStrategy** - обработка YouTube URL через yt-dlp
- **MediaFactory** - фабрика для создания стратегий
- **PlayerAudio** - основной класс для управления воспроизведением
- **PlayerWindow** - графический интерфейс пользователя

## Разработка

### Добавление новых типов медиа

1. Создайте новый класс, наследующийся от `MediaStrategy`
2. Реализуйте метод `resolve()`
3. Добавьте новый тип в `MediaType` enum
4. Обновите `MediaFactory::create()`

### Изменение интерфейса

Файл `res/ui/PlayerWindow.ui` можно редактировать в Qt Designer или Qt Creator.

## Лицензия

MIT License

## Авторы


