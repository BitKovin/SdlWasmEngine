#include "Input.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>
#include <algorithm>
#include <cmath>
#include "glm/glm.hpp"

// Static member definitions for Input.
glm::vec2 Input::MousePos;
glm::vec2 Input::MouseDelta;
std::vector<glm::vec2> Input::MouseDeltas;
int Input::MaxDeltas = 10;
std::unordered_map<std::string, InputAction*> Input::actions;
bool Input::LockCursor = false;
float Input::sensitivity = 0.2f;
glm::vec2 Input::windowCenter;
bool Input::PendingCenterCursor = false;
MouseMoveCalculator* Input::mouseMoveCalculator = nullptr;
SDL_Window* Input::window = nullptr;
SDL_Joystick* Input::joystick = nullptr;
Uint32 Input::lastTime = 0;
float Input::deltaTime = 0.f;
double Input::GameTime = 0.0;

// Helper function to compute distance between two glm::vec2 points.
static float Distance(const glm::vec2& a, const glm::vec2& b) {
    glm::vec2 diff = a - b;
    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}

void Input::Update() {
    // Calculate delta time.
    Uint32 currentTime = SDL_GetTicks();
    deltaTime = (currentTime - lastTime) / 1000.f;
    GameTime = currentTime / 1000.0;
    lastTime = currentTime;

    UpdateMouse();
    UpdateActions();

    if (PendingCenterCursor)
        CenterCursor();

    // Update window center based on current window size.
    int w, h;
    SDL_GetWindowSize(window, &w, &h);
    windowCenter = glm::vec2(w / 2.0f, h / 2.0f);

    // Show or hide the cursor based on LockCursor.
    SDL_ShowCursor(LockCursor ? SDL_DISABLE : SDL_ENABLE);
}

void Input::JoystickCamera() {
    if (joystick) {
        // Define which axes to use for the right thumbstick.
        // These indices may need adjustment depending on your joystick.
        const int axisRightX = 2;
        const int axisRightY = 3;

        // Get normalized axis values (range -1 to 1).
        float axisX = SDL_JoystickGetAxis(joystick, axisRightX) / 32768.f;
        float axisY = SDL_JoystickGetAxis(joystick, axisRightY) / 32768.f;
        // Multiply by deltaTime and a scale factor (1500) and invert Y.
        glm::vec2 stickDelta(axisX, -axisY);
        MouseDelta = MouseDelta + stickDelta * (deltaTime * 1500.f);
    }
}

void Input::UpdateMouse() {
    int x, y;
    SDL_GetMouseState(&x, &y);
    glm::vec2 mousePos(static_cast<float>(x), static_cast<float>(y));

    if (mouseMoveCalculator) {
        MousePos = mousePos;
        MouseDelta = mouseMoveCalculator->GetMouseDelta();
        AddMouseInput(MouseDelta);
        JoystickCamera();
        MouseDelta = MouseDelta * sensitivity;
        return;
    }

    glm::vec2 delta = mousePos - MousePos;
    AddMouseInput(delta);
    JoystickCamera();
    MouseDelta = MouseDelta * sensitivity;

    // If cursor locking is enabled and the window is focused.
    Uint32 flags = SDL_GetWindowFlags(window);
    bool windowActive = (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
    if (LockCursor && windowActive) {
        if (Distance(windowCenter, mousePos) > 0.f) {
            SDL_WarpMouseInWindow(window, static_cast<int>(windowCenter.x), static_cast<int>(windowCenter.y));
            MousePos = windowCenter;
        }
        else {
            MousePos = mousePos;
        }
    }
    else {
        MousePos = mousePos;
    }
}

void Input::AddMouseInput(const glm::vec2& delta) {
    if (static_cast<int>(MouseDeltas.size()) > MaxDeltas)
        MouseDeltas.erase(MouseDeltas.begin());
    MouseDeltas.push_back(delta);

    glm::vec2 sum(0.f, 0.f);
    for (const auto& d : MouseDeltas)
        sum = sum + d;
    MouseDelta = sum / static_cast<float>(MouseDeltas.size());
}

void Input::CenterCursor() {
    Uint32 flags = SDL_GetWindowFlags(window);
    bool windowFocused = (flags & SDL_WINDOW_INPUT_FOCUS) != 0;
    if (windowFocused) {
        SDL_WarpMouseInWindow(window, static_cast<int>(windowCenter.x), static_cast<int>(windowCenter.y));
        MousePos = windowCenter;
        MouseDelta = glm::vec2(0.f, 0.f);
        MouseDeltas.clear();
        PendingCenterCursor = false;
    }
    else {
        PendingCenterCursor = true;
    }
}

void Input::UpdateActions() {
    for (auto& pair : actions) {
        pair.second->Update();
    }
}

InputAction* Input::GetAction(const std::string& actionName) {
    if (actions.find(actionName) == actions.end())
        return nullptr;
    return actions[actionName];
}

InputAction* Input::AddAction(const std::string& actionName) {
    if (actions.find(actionName) == actions.end()) {
        InputAction* action = new InputAction();
        actions[actionName] = action;
        return action;
    }
    return actions[actionName];
}

void Input::RemoveAction(const std::string& actionName)
{
    InputAction* action = GetAction(actionName);

    if (action == nullptr)
        return;

    actions.erase(actionName);

    action->Dispose();
}


// ------------------------
// InputAction Implementation
// ------------------------

InputAction::InputAction() {}

InputAction* InputAction::AddKeyboardKey(SDL_Scancode key) {
    keys.push_back(key);
    return this;
}

InputAction* InputAction::RemoveKeyboardKey(SDL_Scancode key) {
    keys.erase(std::remove(keys.begin(), keys.end(), key), keys.end());
    return this;
}

InputAction* InputAction::AddButton(int button) {
    buttons.push_back(button);
    return this;
}

InputAction* InputAction::RemoveButton(int button) {
    buttons.erase(std::remove(buttons.begin(), buttons.end(), button), buttons.end());
    return this;
}

bool InputAction::Pressed() {
    return pressed;
}

bool InputAction::Released() {
    return released;
}

bool InputAction::Holding() {
    return pressing;
}

bool InputAction::PressedBuffered(float bufferLength) {
    return pressedTime + bufferLength >= Input::GameTime;
}

void InputAction::Update() {
    bool oldPressing = pressing;
    pressed = false;
    released = false;
    pressing = false;

    // Check mouse buttons.
    int mx, my;
    Uint32 mouseState = SDL_GetMouseState(&mx, &my);
    bool newLmb = mouseState & SDL_BUTTON(SDL_BUTTON_LEFT);
    bool newRmb = mouseState & SDL_BUTTON(SDL_BUTTON_RIGHT);
    bool newMmb = mouseState & SDL_BUTTON(SDL_BUTTON_MIDDLE);

    if (LMB && newLmb)
        pressing = true;
    if (RMB && newRmb)
        pressing = true;
    if (MMB && newMmb)
        pressing = true;

    // Check keyboard keys.
    const Uint8* keyboardState = SDL_GetKeyboardState(nullptr);
    for (auto key : keys) {
        if (keyboardState[key])
            pressing = true;
    }

    // Check joystick buttons.
    if (Input::joystick) {
        for (auto button : buttons) {
            if (SDL_JoystickGetButton(Input::joystick, button))
                pressing = true;
        }
    }

    if (pressing && !oldPressing) {
        pressed = true;
        pressedTime = Input::GameTime;
    }
    else if (!pressing && oldPressing) {
        released = true;
    }
}