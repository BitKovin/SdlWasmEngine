@echo off
REM Check if the first argument is empty
if "%~1"=="" (
    set "BUILD_TYPE=Debug"
) else (
    set "BUILD_TYPE=%~1"
    shift
)

set "BUILD_DIR=WASM_%BUILD_TYPE%"

echo Usage: cmake_windows_emscripten.bat [Configuration]
echo Possible configurations: Debug (default), Release, Distribution
echo Generating Makefile for build type "%BUILD_TYPE%" in folder "%BUILD_DIR%"

REM Call cmake with the given parameters. Note that additional arguments are passed using %*
cmake -S . -B "%BUILD_DIR%" -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=%BUILD_TYPE% -DCMAKE_TOOLCHAIN_FILE=%EMSDK%\upstream\emscripten\cmake\Modules\Platform\Emscripten.cmake %*

echo.
echo Compile by running "make -j %NUMBER_OF_PROCESSORS% && node UnitTests.js" in folder "%BUILD_DIR%"
pause
