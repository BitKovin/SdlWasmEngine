#include "EngineMain.h"

EngineMain* EngineMain::MainInstance = nullptr;

UiViewport EngineMain::Viewport;

void EngineMain::InitInputs()
{
    Input::AddAction("forward")->AddKeyboardKey(SDL_GetScancodeFromKey(SDL_KeyCode::SDLK_w));
    Input::AddAction("backward")->AddKeyboardKey(SDL_GetScancodeFromKey(SDL_KeyCode::SDLK_s));
    Input::AddAction("left")->AddKeyboardKey(SDL_GetScancodeFromKey(SDL_KeyCode::SDLK_a));
    Input::AddAction("right")->AddKeyboardKey(SDL_GetScancodeFromKey(SDL_KeyCode::SDLK_d));

    Input::AddAction("jump")->AddKeyboardKey(SDL_GetScancodeFromKey(SDL_KeyCode::SDLK_SPACE));

}