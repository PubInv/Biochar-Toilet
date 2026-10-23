#!/bin/bash
set -e

# Define project directory
PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Setup or install ESP-IDF
if [ -z "$IDF_PATH" ]; then
    export IDF_PATH="$HOME/esp-idf"
fi

if [ ! -d "$IDF_PATH" ]; then
    echo "ESP-IDF not found at $IDF_PATH. Cloning release/v5.3..."
    git clone -b release/v5.3 --recursive https://github.com/espressif/esp-idf.git "$IDF_PATH"
elif [ -d "$IDF_PATH/.git" ]; then
    echo "Updating existing ESP-IDF at $IDF_PATH to release/v5.3..."
    cd "$IDF_PATH"
    git fetch origin || true
    git checkout release/v5.3 || true
    git pull --recurse-submodules || true
    git submodule update --init --recursive || true
fi

echo "Installing ESP-IDF tools..."
cd "$IDF_PATH"
./install.sh esp32h2

echo "Sourcing ESP-IDF environment..."
source "$IDF_PATH/export.sh"

echo "Navigating to biochar induction firmware directory: $PROJECT_DIR"
cd "$PROJECT_DIR"

echo "Cleaning old build files to prevent cache conflicts..."
rm -rf build sdkconfig sdkconfig.old
idf.py fullclean

echo "Setting target to esp32h2..."
idf.py set-target esp32h2

echo "Building biochar induction firmware..."
if [ $# -eq 0 ]; then
    idf.py build
else
    idf.py build "$@"
fi

echo "Build successful."
