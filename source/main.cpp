
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
//#include <SDL2/SDL_image.h>
#if DESKTOP
#else
#include <emscripten.h>
#include <emscripten/html5.h>
#endif 

#include "gl.h"

SDL_Window *window;


#include "render.h"
#include "SoundSystem/SoundManager.hpp"

int main(int argc, char* args[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;

    Input::AddAction("fullscreen");

#if DESKTOP

    flags |= SDL_WINDOW_RESIZABLE;

    Input::AddAction("fullscreen")->AddKeyboardKey(SDL_GetScancodeFromKey(SDL_KeyCode::SDLK_F11));

#endif // DESKTOP


    // Create SDL window
    window = SDL_CreateWindow("Image", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, flags);
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }
    
    SoundManager::Initialize();

    Time::Init();

    auto sound = SoundManager::GetSoundFromPath("GameData/bass_beat.wav");

    sound.Loop = true;

    sound.Play();
    
    printf("start\n");

#if DESKTOP
#else
    // Create OpenGL ES context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        fprintf(stderr, "OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        
    }
#if DESKTOP
    GLenum glewError = glewInit();
    if (glewError != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW: %s\n", glewGetErrorString(glewError));
        
    }
#endif
    
    init_gl();
    
    printf("GL Version={%s}\n", glGetString(GL_VERSION));
    printf("GLSL Version={%s}\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    Input::AddAction("test")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_w));

    update_screen_size(512, 512);

    load_image("GameData/happy_hog.png");
    
    // Set clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    SDL_GL_SetSwapInterval(0);

    // Run main loop
#if DESKTOP
    desktop_render_loop();
#else
    emscripten_set_main_loop(emscripten_render_loop, 0, 1);
#endif
    
    // Clean up
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}

