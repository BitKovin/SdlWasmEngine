@echo off
setlocal enabledelayedexpansion

echo === Activating Emscripten environment ===
call "F:\Tools\emsdk\emsdk_env.bat"
if errorlevel 1 (
  echo Failed to activate build environment.
  exit /b 1
)

echo === [emscripten] Build directory already configured, matching args — skipping reconfigure ===

echo === Building [emscripten] ===
cmake --build "F:\Git\SdlWasmEngine\build\emscripten" --config Release
if errorlevel 1 exit /b 1

exit /b 0
