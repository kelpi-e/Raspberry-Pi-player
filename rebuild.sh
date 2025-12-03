#!/bin/bash

set -e

# === НАСТРОЙКИ ===
PROJ_DIR="$HOME/Raspberry-Pi-player"
BUILD_DIR="$PROJ_DIR/build"
APP="$BUILD_DIR/untitled3"
ROTATION=0

# === СБОРКА ===
echo "[1/3] Удаление старой сборки..."
rm -rf "$BUILD_DIR"

echo "[2/3] Создание build/..."
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

echo "[3/3] CMake + Make..."
cmake ..
make -j2

# === ЗАПУСК ===
echo "Сборка завершена."

if [ ! -x "$APP" ]; then
    echo "Ошибка: бинарник $APP не найден"
    exit 1
fi

echo "Запуск приложения с поворотом $ROTATION°..."

export QT_QPA_PLATFORM=linuxfb
export QT_QPA_FB_ROTATION=$ROTATION

"$APP"
