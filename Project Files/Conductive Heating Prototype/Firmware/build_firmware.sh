#!/bin/bash
set -e

# Define project directory
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Setup or install ESP-IDF
if [ -z "$IDF_PATH" ]; then
    export IDF_PATH="$HOME/esp-idf"
fi

if [ ! -d "$IDF_PATH" ]; then
    echo "ESP-IDF not found at $IDF_PATH. Cloning..."
    git clone -b v5.2.1 --recursive https://github.com/espressif/esp-idf.git "$IDF_PATH"
fi

echo "Installing ESP-IDF tools..."
cd "$IDF_PATH"
./install.sh esp32h2

echo "Sourcing ESP-IDF environment..."
source "$IDF_PATH/export.sh"

echo "Navigating to firmware directory: $PROJECT_DIR"
cd "$PROJECT_DIR"

echo "Cleaning old build files to prevent cache conflicts..."
rm -rf build
idf.py fullclean

echo "Setting target to esp32h2..."
idf.py set-target esp32h2

echo "Building firmware..."
idf.py build

echo "Build successful."
