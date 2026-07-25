@echo off
setlocal enabledelayedexpansion

echo === Activating GDK environment ===
call "C:\Program Files (x86)\Microsoft GDK\Command Prompts\GamingDesktopVars.cmd" GamingDesktopVS2022
if errorlevel 1 (
  echo Failed to activate build environment.
  exit /b 1
)

echo === Configuring [windows-gdk] ===
cmake -S "F:\Git\SdlWasmEngine" -B "F:\Git\SdlWasmEngine\build\windows_gdk" -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE="F:\Git\SdlWasmEngine\cmake\toolchains\msvc.cmake" -DGAME_NAME="ShooterGame" -DSKIP_GAMEDATA_FINALIZE=1 -DWINDOWS_GDK=1
if errorlevel 1 exit /b 1

echo === Building [windows-gdk] ===
cmake --build "F:\Git\SdlWasmEngine\build\windows_gdk" --config Release
if errorlevel 1 exit /b 1

exit /b 0
