#if __EMSCRIPTEN__ //so it doesn't results in errors on other platforms

#define DISTRIBUTION

#define _HAS_STD_BYTE 0
#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_sdl2.h"
#include "../gl.h"
#include <deque>
#include <algorithm>
#include <array>

#include "../EngineMain.h"

// Global variables

#include "PlatformWindowData.h"
using namespace PlatformWindowData;

EngineMain* engine = nullptr;
std::deque<vec2> delta_history;
const size_t history_size = 3;

// Function declarations
void update_screen_size(int w, int h);
void InitImGui();
void emscripten_render_loop();

// Function implementations
void update_screen_size(int w, int h) {
    SDL_SetWindowSize(window, w, h);
}

void InitImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(window, glContext);
    ImGui_ImplOpenGL3_Init();
}

static EM_BOOL on_canvas_focus(int eventType, const EmscriptenFocusEvent* e, void* userData) {
    EngineMain::MainInstance->Paused = false;
    return EM_TRUE; // we handled it
}

static EM_BOOL on_canvas_blur(int eventType, const EmscriptenFocusEvent* e, void* userData) {
    EngineMain::MainInstance->Paused = true;
    return EM_TRUE;
}

void emscripten_render_loop() {
    Input::PendingMouseDelta = vec2(0, 0);
    SDL_Event event;
    Input::StartEventsFrame();
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        switch (event.type) {
        case SDL_WINDOWEVENT:
            if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                update_screen_size(event.window.data1, event.window.data2);
            }
            break;
        case SDL_MOUSEMOTION:
            Input::PendingMouseDelta += vec2(event.motion.xrel, event.motion.yrel) * 3.0f * vec2(Camera::AspectRatio,1);

            break;
        default:
            break;
        }

        Input::ReceiveSdlEvent(event);

    }

    engine->MainLoop();
}

// Main function
int main(int argc, char* args[]) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 2);

    int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
    window = SDL_CreateWindow("Image", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, flags);
    if (!window) {
        fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }


    TTF_Init();
    glContext = SDL_GL_CreateContext(window);
    if (!glContext) {
        fprintf(stderr, "OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    InitImGui();
    SDL_SetHintWithPriority(SDL_HINT_MOUSE_RELATIVE_MODE_CENTER, "1", SDL_HINT_OVERRIDE);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    printf("GL Version={%s}\n", glGetString(GL_VERSION));
    printf("GLSL Version={%s}\n", glGetString(GL_SHADING_LANGUAGE_VERSION));


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    SDL_GL_SetSwapInterval(0);

    engine = new EngineMain(window);

#ifndef  __EMSCRIPTEN_PTHREADS__

    engine->asyncGameUpdate = false;

#endif // ! __EMSCRIPTEN_PTHREADS__

    EngineMain::MainInstance = engine;

    emscripten_sleep(300);//some time for js bounce back for correct start resolution

    engine->Init();

    Input::AddAction("fullscreen")->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_F11));

    EmscriptenFullscreenStrategy strategy;
    strategy.scaleMode = EMSCRIPTEN_FULLSCREEN_SCALE_STRETCH;
    strategy.filteringMode = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
    strategy.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_STDDEF;
    strategy.canvasResizedCallback = 0;
    //emscripten_enter_soft_fullscreen("canvas", &strategy); //not an error

    //emscripten_set_focus_callback("#canvas", nullptr, EM_FALSE, on_canvas_focus);
    //emscripten_set_blur_callback("#canvas", nullptr, EM_FALSE, on_canvas_blur);

    emscripten_sleep(300);//some time for js bounce back for correct start resolution

    emscripten_set_main_loop(emscripten_render_loop, 0, 1);

    delete engine;

    return 0;
}

#endif