#!/bin/bash
set -e

REPO_URL="https://github.com/kelpi-e/Raspberry-Pi-player.git"
BRANCHES=("main" "web-interface")

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

PROJECT_FOLDER="Raspberry-Pi-player"

echo "Starting branches cloning..."
echo "========================================"

# --- Создание папки проекта ---
if [ ! -d "$PROJECT_FOLDER" ]; then
    mkdir -p "$PROJECT_FOLDER"
fi

cd "$PROJECT_FOLDER" || exit 1

# --- Клонирование веток ---
for branch in "${BRANCHES[@]}"; do
    if [ ! -d "$branch" ]; then
        git clone --branch "$branch" --single-branch "$REPO_URL" "$branch"
    fi
done

echo -e "${GREEN}Cloning completed${NC}"

# ==================================================
#                C++ СБОРКА
# ==================================================
echo "Installing C++ dependencies..."

if command -v pacman &>/dev/null; then
    sudo pacman -S --needed qt6-base qt6-multimedia taglib cmake
elif command -v apt &>/dev/null; then
    sudo apt update
    sudo apt install -y qt6-base-dev qt6-multimedia-dev libtag1-dev cmake
else
    echo "Unsupported package manager. Install Qt6 and TagLib manually."
    exit 1
fi

echo -e "\n${GREEN}Building C++ project...${NC}"

ROTATION=${1:-0}

if ! [[ "$ROTATION" =~ ^[0-9]+$ ]]; then
    echo -e "${RED}Rotation must be number (0/90/180/270)${NC}"
    exit 1
fi

rm -rf build

cmake -S main -B build
cmake --build build -j$(nproc)

APP_PATH="build/Raspberry-Pi-player"

if [ ! -x "$APP_PATH" ]; then
    echo -e "${RED}Binary not found: $APP_PATH${NC}"
    exit 1
fi

echo -e "${GREEN}C++ build completed${NC}"

# ==================================================
#                PYTHON ОКРУЖЕНИЕ
# ==================================================

echo -e "\n${GREEN}Setting up Python environment...${NC}"

if [ ! -d "web-interface" ]; then
    echo -e "${RED}web-interface folder not found${NC}"
    exit 1
fi

cd web-interface || exit 1

python3 -m venv .venv
source .venv/bin/activate

pip install --upgrade pip
pip install -r requirements.txt

deactivate
cd ..

echo -e "${GREEN}Environment setup completed${NC}"

echo
echo "Run C++ app:"
echo "export QT_QPA_PLATFORM=linuxfb"
echo "./$APP_PATH $ROTATION"

echo
echo "Activate Python env:"
echo "cd web-interface && source .venv/bin/activate"

echo -e "\n${GREEN}Creating run scripts...${NC}"

# --- run_cpp.sh ---
cat << 'EOF' > run_cpp.sh
#!/bin/bash
set -e

APP="build/Raspberry-Pi-player"
ROTATION=${1:-0}

if ! [[ "$ROTATION" =~ ^[0-9]+$ ]]; then
    echo "Rotation must be number (0/90/180/270)"
    exit 1
fi

if [ ! -x "$APP" ]; then
    echo "Binary not found. Build project first."
    exit 1
fi

export QT_QPA_PLATFORM=linuxfb
./"$APP" "$ROTATION"
EOF

chmod +x run_cpp.sh

# --- run_python.sh ---
cat << 'EOF' > run_python.sh
#!/bin/bash
set -e

cd web-interface || {
    echo "web-interface folder not found"
    exit 1
}

if [ ! -d ".venv" ]; then
    echo "Virtual environment not found. Run setup first."
    exit 1
fi

source .venv/bin/activate
python main.py
EOF

chmod +x run_python.sh

echo -e "${GREEN}Run scripts created successfully!${NC}"