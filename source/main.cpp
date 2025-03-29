
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

void update_screen_size(int w, int h)
{
    glViewport(0, 0, w, h);
    SDL_SetWindowSize(window, w, h);
}

#include "render.h"
#include "SoundSystem/SoundManager.hpp"

#include "EngineMain.h"


EngineMain* engine = nullptr;

SDL_GLContext glContext;


#if WINDOWS

#include <dinput.h>
#include <SDL2/SDL_syswm.h> // Add this header for window system info
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

// Globals
LPDIRECTINPUT8 g_pDI = nullptr; // Declare DirectInput interface
LPDIRECTINPUTDEVICE8 g_pMouse = nullptr;

bool InitDirectInput(SDL_Window* sdlWindow) {
    // Get native Windows handle from SDL
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(sdlWindow, &wmInfo)) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Failed to get window handle: %s", SDL_GetError());
        return false;
    }
    HWND hwnd = wmInfo.info.win.window;

    // Initialize DirectInput
    HRESULT hr = DirectInput8Create(
        GetModuleHandle(nullptr),
        DIRECTINPUT_VERSION,
        IID_IDirectInput8,
        (void**)&g_pDI,
        nullptr
    );

    if (FAILED(hr)) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "DirectInput8Create failed: %08X", hr);
        return false;
    }

    // Create mouse device
    hr = g_pDI->CreateDevice(GUID_SysMouse, &g_pMouse, nullptr);
    if (FAILED(hr)) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "CreateDevice failed: %08X", hr);
        return false;
    }

    // Set data format for relative mouse movement
    hr = g_pMouse->SetDataFormat(&c_dfDIMouse2);
    if (FAILED(hr)) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "SetDataFormat failed: %08X", hr);
        return false;
    }

    // Set cooperative level (non-exclusive + background access)
    hr = g_pMouse->SetCooperativeLevel(
        hwnd,
        DISCL_NONEXCLUSIVE | DISCL_BACKGROUND
    );

    if (FAILED(hr)) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "SetCooperativeLevel failed: %08X", hr);
        return false;
    }

    // Acquire the mouse
    hr = g_pMouse->Acquire();
    if (FAILED(hr)) {
        SDL_LogError(SDL_LOG_CATEGORY_INPUT, "Acquire failed: %08X", hr);
        return false;
    }

    // Hide SDL cursor
    SDL_ShowCursor(SDL_DISABLE);

    return true;
}



vec2 ReadMouseDelta() {

    DIMOUSESTATE2 mouseState;
    vec2 mouseDelta = vec2(0, 0);

    if (g_pMouse == nullptr)
        return vec2();

    HRESULT hr = g_pMouse->GetDeviceState(sizeof(DIMOUSESTATE2), &mouseState);
    if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
        // Reacquire the mouse if focus was lost (e.g., alt-tab)
        g_pMouse->Acquire();
    }
    else if (SUCCEEDED(hr)) {
        // DirectInput provides relative deltas in `lX` and `lY`
        mouseDelta.x = static_cast<float>(mouseState.lX);
        mouseDelta.y = static_cast<float>(mouseState.lY);
    }

    return mouseDelta;

}

void ShutdownDirectInput() {
    if (g_pMouse) {
        g_pMouse->Unacquire();
        g_pMouse->Release();
        g_pMouse = nullptr;
    }
    if (g_pDI) {
        g_pDI->Release();
        g_pDI = nullptr;
    }
}

#endif // WINDOWS



void desktop_render_loop()
{

    SDL_Event event;
    int quit = 0;



    while (!quit) {

        Input::PendingMouseDelta = vec2();

#if WINDOWS
        Input::PendingMouseDelta = ReadMouseDelta();
#endif

        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                quit = 1;

            if (event.type == SDL_MOUSEMOTION)
            {
#if WINDOWS == FALSE
                Input::PendingMouseDelta += vec2(event.motion.xrel, event.motion.yrel);
#endif
            }

        }
        engine->MainLoop();

    }
}


void emscripten_render_loop()
{

    Input::PendingMouseDelta = vec2();

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                update_screen_size(event.window.data1, event.window.data2);
            }
            break;
        default:
            break;
        }

        if (event.type == SDL_MOUSEMOTION)
        {

            Input::PendingMouseDelta = vec2(event.motion.xrel, event.motion.yrel);

        }

    }

    engine->MainLoop();

}

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
   
#if WINDOWS

    InitDirectInput(window);

#endif
    
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
    //SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_WARP, "1", SDL_HINT_OVERRIDE);
    
    printf("GL Version={%s}\n", glGetString(GL_VERSION));
    printf("GLSL Version={%s}\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    Input::AddAction("test")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_w));;
    
    // Set clear color
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    SDL_GL_SetSwapInterval(0);

    SDL_SetRelativeMouseMode(SDL_TRUE);

    engine = new EngineMain(window);

    EngineMain::MainInstance = engine;

    engine->Init();

    // Run main loop
#if DESKTOP
    desktop_render_loop();
#else
    emscripten_set_main_loop(emscripten_render_loop, 0, 1);
#endif
    
#if  WINDOWS



#endif //  WINDOWS


    // Clean up
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    return 0;
}

