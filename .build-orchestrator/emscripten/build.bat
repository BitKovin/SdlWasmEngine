@echo off
setlocal enabledelayedexpansion

echo === Activating Emscripten environment ===
call "F:\Tools\emsdk\emsdk_env.bat"
if errorlevel 1 (
  echo Failed to activate build environment.
  exit /b 1
)

echo === Configuring [emscripten] ===
emcmake cmake -S "F:\Git\SdlWasmEngine" -B "F:\Git\SdlWasmEngine\build\emscripten" -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DGAME_NAME="ShooterGame" -DSKIP_GAMEDATA_FINALIZE=1
if errorlevel 1 exit /b 1

echo === Building [emscripten] ===
cmake --build "F:\Git\SdlWasmEngine\build\emscripten" --config Release
if errorlevel 1 exit /b 1

exit /b 0
