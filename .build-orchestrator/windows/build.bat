@echo off
setlocal enabledelayedexpansion

echo === Activating Visual Studio environment ===
call "D:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 (
  echo Failed to activate build environment.
  exit /b 1
)

echo === Configuring [windows] ===
cmake -S "F:\Git\SdlWasmEngine" -B "F:\Git\SdlWasmEngine\build\windows" -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="F:\Git\SdlWasmEngine\cmake\toolchains\msvc.cmake" -DGAME_NAME="ShooterGame" 
if errorlevel 1 exit /b 1

echo === Building [windows] ===
cmake --build "F:\Git\SdlWasmEngine\build\windows" --config Release
if errorlevel 1 exit /b 1

exit /b 0
