#!/bin/bash

set -e

# === НАСТРОЙКИ ===
PROJ_DIR="$HOME/Raspberry-Pi-player"
BUILD_DIR="$PROJ_DIR/build"
APP="$BUILD_DIR/Raspberry-Pi-player"

# === ПОВОРОТ ===
ROTATION=${1:-0}   # если аргумент не передан, по умолчанию 0

# Проверка, что ROTATION — число
if ! [[ "$ROTATION" =~ ^[0-9]+$ ]]; then
    echo "Ошибка: аргумент поворота должен быть числом (0, 90, 180, 270)"
    exit 1
fi

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

"$APP" "$ROTATION"  # передаём угол как аргумент
