#pragma once
#if DESKTOP
#include <GL/glew.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL2/SDL_opengles2.h>
#include <GLES3/gl31.h>
#endif
