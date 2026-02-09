#!/bin/bash

REPO_URL="https://github.com/kelpi-e/Raspberry-Pi-player.git"
BRANCHES=("main" "web-interface")

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo "Starting branches cloning from repository..."
echo "URL: $REPO_URL"
echo "========================================"


PROJECT_FOLDER="Raspberry-Pi-player"

if [ ! -d "$PROJECT_FOLDER" ]; then
    echo "Creating project folder: $PROJECT_FOLDER"
    if mkdir -p "$PROJECT_FOLDER"; then
        echo "Project folder created successfully"
    else
        echo -e "${RED}Failed to create project folder${NC}"
        exit 1
    fi
else
    echo -e "${YELLOW}Project folder '$PROJECT_FOLDER' already exists${NC}"
fi

cd "$PROJECT_FOLDER" || {
    echo -e "${RED}Failed to change to project directory${NC}"
    exit 1
}

for branch in "${BRANCHES[@]}"; do
    folder_name=$(echo "$branch" | sed 's/[^a-zA-Z0-9._-]/_/g')

    echo -e "\n${GREEN}Processing branch: $branch${NC}"

    if [ -d "$folder_name" ]; then
        echo -e "${YELLOW}Folder '$folder_name' already exists. Skipping...${NC}"
        continue
    fi

    echo "Cloning branch '$branch' to folder '$folder_name'..."

    if git clone --branch "$branch" --single-branch "$REPO_URL" "$folder_name" 2>/dev/null; then
        echo "Branch '$branch' cloned successfully to '$folder_name'"

        # Remove .git folder to keep only the files (optional)
        # rm -rf "$folder_name/.git"

    else
        echo -e "${RED}Error cloning branch '$branch'${NC}"
        echo "Please verify branch name and repository access"

        if [ -d "$folder_name" ]; then
            echo "Cleaning up partial clone..."
            rm -rf "$folder_name"
        fi
    fi
done

echo -e "\n${GREEN}========================================"
echo "Cloning completed!"
echo "========================================"
echo -e "${NC}Cloned branches:"
for branch in "${BRANCHES[@]}"; do
    folder_name=$(echo "$branch" | sed 's/[^a-zA-Z0-9._-]/_/g')
    if [ -d "$folder_name" ]; then
        echo "  - $branch → ./$PROJECT_FOLDER/$folder_name/"
    else
        echo -e "${RED}  $branch → Failed to clone${NC}"
    fi
done

