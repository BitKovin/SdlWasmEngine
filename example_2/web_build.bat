REM emsdk activate

call emsdk activate

emcc source/main.c  -o main.html -s USE_SDL=2 -sUSE_SDL_IMAGE=2 -sSDL2_IMAGE_FORMATS=["png"] -sFULL_ES2=1 -s ALLOW_MEMORY_GROWTH=1 --preload-file assets