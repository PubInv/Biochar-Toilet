@echo off
setlocal

:: Define project directory
set "PROJECT_DIR=%~dp0"

:: Setup or install ESP-IDF
if "%IDF_PATH%"=="" set "IDF_PATH=%USERPROFILE%\esp-idf"

if not exist "%IDF_PATH%\install.bat" (
    echo ESP-IDF not found at %IDF_PATH%. Cloning and installing...
    git clone -b v5.2.1 --recursive https://github.com/espressif/esp-idf.git "%IDF_PATH%"
    cd /d "%IDF_PATH%"
    call install.bat esp32h2
)

echo Sourcing ESP-IDF environment...
cd /d "%IDF_PATH%"
call export.bat

echo Navigating to firmware directory: %PROJECT_DIR%
cd /d "%PROJECT_DIR%"

echo Setting target to esp32h2...
idf.py set-target esp32h2

echo Building firmware...
idf.py build

echo Build successful.
endlocal
