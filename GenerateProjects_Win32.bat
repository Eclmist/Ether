@echo off

:: ============== STANDALONE CONFIG (WIN32) ============== ::
echo Generating Visual Studio solutions for Ether
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

echo Ether solution generated successfully

:: ================== TOOLMODE CONFIG ================== ::

echo Generating Visual Studio solutions for Ether (Toolmode)

cmake -D CONFIGURE_AS_TOOLMODE:BOOL=1 -B build\win32

if %errorlevel% neq 0 (
    echo CMake exited with error code %errorlevel%
    echo Projects generation failed
    color CF
    pause
    exit
)

echo Ether (toolmode) solution generated successfully

color 2F
echo All configurations generated successfully
PAUSE

