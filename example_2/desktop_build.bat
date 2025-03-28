@echo off
set include_paths= /I"C:\SDL2\access_path" /I"C:\glew\glew-2.1.0\include"

set lib_path= "C:\SDL2\lib\x64"

set common_compiler_flags= -MTd -nologo -fp:fast -Gm- -GR- -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189  -wd4244 -wd4505 -wd4026 -wd4028 -DDEBUG=1 -DWIN32_PROGAM=1 -DDESKTOP=1 -FC -Z7 
set common_linker_flags= -incremental:no -opt:ref  user32.lib opengl32.lib glew32.lib SDL2main.lib SDL2.lib SDL2_image.lib SDL2_mixer.lib gdi32.lib winmm.lib imm32.lib version.lib user32.lib shell32.lib dwmapi.lib

if not exist build mkdir build
pushd build

cl  %common_compiler_flags% /Fe:win32_main.exe ..\source\main.c -Fmwin32_main.map %include_paths% /link /LIBPATH:C:\SDL2\lib\x64 /LIBPATH:C:\glew\glew-2.1.0\lib\Release\x64 %common_linker_flags% /SUBSYSTEM:WINDOWS  





