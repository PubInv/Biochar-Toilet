#!/bin/bash
set -e

# Define project directory
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Setup or install ESP-IDF
export IDF_PATH="$HOME/esp-idf"

if [ ! -d "$IDF_PATH" ]; then
    echo "ESP-IDF not found at $IDF_PATH. Cloning and installing..."
    git clone -b v5.2.1 --recursive https://github.com/espressif/esp-idf.git "$IDF_PATH"
    cd "$IDF_PATH"
    ./install.sh esp32h2
fi

echo "Sourcing ESP-IDF environment..."
source "$IDF_PATH/export.sh"

echo "Navigating to firmware directory: $PROJECT_DIR"
cd "$PROJECT_DIR"

echo "Setting target to esp32h2..."
idf.py set-target esp32h2

echo "Building firmware..."
idf.py build

echo "Build successful."
