@echo off
setlocal

:: Define project directory
set "PROJECT_DIR=%~dp0"

:: Setup or install ESP-IDF
if "%IDF_PATH%"=="" set "IDF_PATH=%USERPROFILE%\esp-idf"

if not exist "%IDF_PATH%\install.bat" (
    echo ESP-IDF not found at %IDF_PATH%. Cloning...
    git clone -b v5.2.1 --recursive https://github.com/espressif/esp-idf.git "%IDF_PATH%"
)

echo Installing ESP-IDF tools...
call "%IDF_PATH%\install.bat" esp32h2

echo Sourcing ESP-IDF environment...
call "%IDF_PATH%\export.bat"

echo Navigating to firmware directory: %PROJECT_DIR%
cd /d "%PROJECT_DIR%"

echo Cleaning old build files to prevent cache conflicts...
idf.py fullclean

echo Setting target to esp32h2...
idf.py set-target esp32h2

echo Building firmware...
idf.py build

echo Build successful.
endlocal
