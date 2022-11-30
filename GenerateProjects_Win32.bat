@echo off

:: ============== STANDALONE CONFIG (WIN32) ============== ::
echo Generating Visual Studio solutions for Ether
cmake -D CONFIGURE_AS_TOOLMODE:BOOL=0 -B build\win32 -DUSE_PRECOMPILED_HEADERS=ON
if %errorlevel% neq 0 (
    echo CMake exited with error code %errorlevel%
    echo Projects generation failed
    color CF
    pause
    exit
)

echo Creating Symbolic Links for Data
if not exist ".\build\win32\Data" md ".\build\win32\Data"
if not exist ".\build\win32\Data\shaders" mklink /J ".\build\win32\Data\shaders" ".\src\graphics\shaders"

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

cmake -D CONFIGURE_AS_TOOLMODE:BOOL=1 -B build\toolmode

if %errorlevel% neq 0 (
    echo CMake exited with error code %errorlevel%
    echo Projects generation failed
    color CF
    pause
    exit
)

echo Creating Symbolic Links for Data
if not exist ".\build\toolmode\Data" md ".\build\toolmode\Data"
if not exist ".\build\toolmode\Data\shaders" mklink /J ".\build\toolmode\Data\shaders" ".\src\graphics\shaders"

echo Ether (toolmode) solution generated successfully

color 2F
echo All configurations generated successfully
PAUSE

