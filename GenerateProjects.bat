@echo off

echo Generating Visual Studio solutions for Ether
cmake -B build -S .

if %errorlevel% neq 0 (
    echo CMake exited with error code %errorlevel%
    echo Projects generation failed
    color CF
    pause
    exit
)

echo Creating Symbolic Links for Data
if not exist ".\build\Data" md ".\build\Data"
if not exist ".\build\Data\shaders" mklink /J ".\build\Data\shader" ".\src\graphic\shader"

if %errorlevel% neq 0 (
    echo Failed to create symlink with error code %errorlevel%
    echo Projects generation failed
    color CF
    pause
    exit
)
color 2F
echo Projects generated successfully

PAUSE