@echo off

:: ============== WINDOWS ============== ::
echo Generating Visual Studio solutions for Ether (Windows)
cmake -B build\win32
if %errorlevel% neq 0 (
    echo CMake exited with error code %errorlevel%
    echo Projects generation failed
    color CF
    pause
    exit
)

echo Creating Symbolic Links for Data
if not exist ".\build\win32\Data" md ".\build\win32\Data"
if not exist ".\build\win32\Data\shader" mklink /J ".\build\win32\Data\shader" ".\src\graphic\shader"

if %errorlevel% neq 0 (
    echo Failed to create symlink with error code %errorlevel%
    echo Projects generation failed
    color CF
    pause
    exit
)

echo Windows solutions generated successfully

color 2F
echo Projects generated successfully
PAUSE