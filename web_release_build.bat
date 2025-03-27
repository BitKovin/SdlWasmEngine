REM emsdk activate

call emsdk activate

emcc source/main.cpp  -o main.html -s USE_SDL=2 -sUSE_SDL_IMAGE=2 -sUSE_SDL_MIXER=2 -sSDL2_IMAGE_FORMATS=["png"] -sFULL_ES3=1 -sSTACK_SIZE=500000 -s ALLOW_MEMORY_GROWTH=1 -O2 --preload-file assets

REM remove GL_DEBUG and GL_ASSERTIONS FOR RELEASE