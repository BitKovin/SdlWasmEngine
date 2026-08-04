@echo off
setlocal enabledelayedexpansion

echo === Activating GDK environment ===
call "C:\Program Files (x86)\Microsoft GDK\Command Prompts\GamingDesktopVars.cmd" GamingDesktopVS2022
if errorlevel 1 (
  echo Failed to activate build environment.
  exit /b 1
)

echo === [windows-gdk] Build directory already configured, matching args — skipping reconfigure ===

echo === Building [windows-gdk] ===
cmake --build "F:\Git\SdlWasmEngine\build\windows_gdk" --config Release
if errorlevel 1 exit /b 1

exit /b 0
