@echo off
setlocal

:: Define project directory
set "PROJECT_DIR=%~dp0"

:: Setup or install ESP-IDF
if "%IDF_PATH%"=="" set "IDF_PATH=%USERPROFILE%\esp-idf"

if not exist "%IDF_PATH%\install.bat" (
    echo ESP-IDF not found at %IDF_PATH%. Cloning release/v5.3...
    git clone -b release/v5.3 --recursive https://github.com/espressif/esp-idf.git "%IDF_PATH%"
) else if exist "%IDF_PATH%\.git" (
    echo Updating existing ESP-IDF at %IDF_PATH% to release/v5.3...
    pushd "%IDF_PATH%"
    git fetch origin
    git checkout release/v5.3
    git pull --recurse-submodules
    git submodule update --init --recursive
    popd
)

echo Installing ESP-IDF tools...
call "%IDF_PATH%\install.bat" esp32h2

echo Sourcing ESP-IDF environment...
call "%IDF_PATH%\export.bat"

echo Navigating to biochar induction firmware directory: %PROJECT_DIR%
cd /d "%PROJECT_DIR%"

echo Cleaning old build files to prevent cache conflicts...
if exist build rmdir /s /q build
if exist sdkconfig del /q sdkconfig
if exist sdkconfig.old del /q sdkconfig.old
idf.py fullclean

echo Setting target to esp32h2...
idf.py set-target esp32h2

echo Building biochar induction firmware...
if "%~1"=="" (
    idf.py build
) else (
    idf.py build %*
)

echo Build successful.
endlocal
