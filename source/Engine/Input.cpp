#include "Input.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>
#include <algorithm>
#include <cmath>
#include "glm.h"
#include "Camera.h"
#include "UI/UiManager.h"
#include "UI/RmlUi/RmlUiContext.h"

#include "Time.hpp"

#include "EngineMain.h"
#include <Logger.hpp>

// Static member definitions for Input.
glm::vec2 Input::MousePos;
glm::vec2 Input::MouseDelta;
glm::vec2 Input::PendingMouseDelta;
std::vector<glm::vec2> Input::MouseDeltas;
int Input::MaxDeltas = 1;
std::unordered_map<std::string, InputAction*> Input::actions;
std::unordered_map<int, TouchEvent> Input::TouchActions;
bool Input::IsScrenTouched = false;
bool Input::LockCursor = false;
float Input::sensitivity = 0.22f;
glm::vec2 Input::windowCenter;
bool Input::PendingCenterCursor = false;
bool Input::PendingWindowStateReset = false;
MouseMoveCalculator* Input::mouseMoveCalculator = nullptr;
SDL_Window* Input::window = nullptr;
SDL_Joystick* Input::joystick = nullptr;

// Event-driven state
std::unordered_set<SDL_Scancode> Input::activeKeys;
bool Input::lmbHeld        = false;
bool Input::rmbHeld        = false;
bool Input::mmbHeld        = false;
std::unordered_set<int> Input::activeJoystickButtons;
float Input::leftTriggerAxis  = 0.f;
float Input::rightTriggerAxis = 0.f;

InputAction InputAction::NullAction = InputAction();

// Helper function to compute distance between two glm::vec2 points.
static float Distance(const glm::vec2& a, const glm::vec2& b) {
    glm::vec2 diff = a - b;
    return std::sqrt(diff.x * diff.x + diff.y * diff.y);
}

void Input::Update() {

    UpdateMousePosition();

    // Calculate delta time. 
    Uint32 currentTime = SDL_GetTicks();

    UpdateMouse();
    UpdateActions();

    if (PendingCenterCursor)
        CenterCursor();

    
    windowCenter = glm::vec2(EngineMain::MainInstance->ScreenSize.x / 2.0f, EngineMain::MainInstance->ScreenSize.y / 2.0f);

    // Show or hide the cursor based on LockCursor.
    SDL_ShowCursor(LockCursor ? SDL_DISABLE : SDL_ENABLE);
    SDL_CaptureMouse(LockCursor ? SDL_TRUE : SDL_FALSE);
}

void Input::UpdateMousePosition()
{
    int x, y;

    int w, h;

    SDL_GetMouseState(&x, &y);

    SDL_GetWindowSize(EngineMain::MainInstance->Window, &w, &h);



    MouseDelta = PendingMouseDelta / 5.0f * sensitivity * -1.0f;

}



void Input::JoystickCamera() {
    // Initialize joystick if not already opened
    if (joystick == nullptr) {
        int joysticks = SDL_NumJoysticks();
        if (joysticks > 0) {
            joystick = SDL_JoystickOpen(0);
            if (joystick == nullptr) {
                SDL_Log("Error opening joystick 0: %s", SDL_GetError());
            }
        }
    }

    // If joystick is open, check if it's still attached
    if (joystick) {
        if (!SDL_JoystickGetAttached(joystick)) {
            // Joystick was disconnected; close and reset
            SDL_JoystickClose(joystick);
            joystick = nullptr;
            activeJoystickButtons.clear();
            leftTriggerAxis  = 0.f;
            rightTriggerAxis = 0.f;
            SDL_Log("Joystick disconnected. Closed and reset reference.");
            return;
        }

        // Read stick position (invert X and Y as needed)
        vec2 stickDelta = GetRightStickPosition() * vec2(-1, 1);
        // Apply camera movement based on stick input
        MouseDelta += stickDelta * (static_cast<float>(Time::DeltaTime) * 200.0f);
    }
}

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>

EM_JS(void, release_cursor_js, (), {
    // Ensure that the pointer lock is active on the canvas
    if (document.pointerLockElement === Module['canvas']) {
      document.exitPointerLock();
    }

    });

EM_JS(void, lock_cursor_js, (), {
  if (Module['canvas']) {
    Module['canvas'].requestPointerLock();
  }
    });

#endif // __EMSCRIPTEN


 
void Input::UpdateMouse() {


    if (IsScrenTouched)
    {
        MouseDelta = vec2(0);
    }

    JoystickCamera();

    bool currentMouse = SDL_GetRelativeMouseMode();

    if (LockCursor != currentMouse) 
    {
        if (LockCursor)
        {
            SDL_SetRelativeMouseMode(SDL_TRUE);
            Logger::Info("locking cursor");
            PendingWindowStateReset = true;

#ifdef __EMSCRIPTEN__
            lock_cursor_js();
#endif // 
        }
        else
        {
#ifdef __EMSCRIPTEN__
            release_cursor_js();
#endif // 
            SDL_SetRelativeMouseMode(SDL_FALSE);
            Logger::Info("unlocking cursor");
            PendingWindowStateReset = true;
        }
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

void Input::StartEventsFrame()
{

    vector<int> toRemove;

    IsScrenTouched = false;

    MouseScrollDelta = 0;

    for (auto& action : TouchActions)
    {

        if (action.second.released)
        {
            toRemove.push_back(action.first);
        }

        action.second.pressed = false;
        action.second.delta = vec2();

        if (action.second.id >= 10)
        {
            IsScrenTouched = true;
        }

    }

    for (int i : toRemove)
    {
        TouchActions.erase(i);
    }


    float screenToViewportRatio = Camera::ScreenHeight / UiManager::GetScaledUiHeight();



    glm::vec2 mousePos = Input::MousePos / screenToViewportRatio; // assume scaled to screen

    TouchEvent mouseTouchEvent;
    mouseTouchEvent.id = 1;
    mouseTouchEvent.delta = MouseDelta;
    mouseTouchEvent.position = mousePos;
    mouseTouchEvent.pressed = GetAction("click")->Pressed();
    mouseTouchEvent.released = GetAction("click")->Released();

    TouchActions[1] = mouseTouchEvent;

}

void Input::ReceiveSdlEvent(SDL_Event event)
{

	EngineMain::MainInstance->RmlContext->ProcessEvent(event);

    vec2 UiScreenSize = EngineMain::Viewport.GetSize();

    int w, h;
    SDL_GetWindowSize(EngineMain::MainInstance->Window, &w, &h);

    // ------------------------------------------------------------------
    // Keyboard — event-driven, repeat events are intentionally ignored.
    // SDL fires SDL_KEYDOWN repeatedly while a key is held (OS key-repeat).
    // We only act on the first physical press (repeat == 0) and the release.
    // ------------------------------------------------------------------
    if (event.type == SDL_KEYDOWN)
    {
        if (event.key.repeat == 0)
        {
            // First physical press — mark as held.
            activeKeys.insert(event.key.keysym.scancode);
        }
        // repeat != 0 → key-repeat spam, ignore entirely.
    }
    else if (event.type == SDL_KEYUP)
    {
        activeKeys.erase(event.key.keysym.scancode);
    }
    else if (event.type == SDL_MOUSEMOTION)
    {
        MousePos.x = static_cast<float>(event.motion.x);
        MousePos.y = static_cast<float>(event.motion.y);

    }

    // ------------------------------------------------------------------
    // Mouse buttons — event-driven.
    // Mouse *movement* is intentionally left to the external handler.
    // ------------------------------------------------------------------
    else if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        if (event.button.button == SDL_BUTTON_LEFT)   lmbHeld = true;
        if (event.button.button == SDL_BUTTON_RIGHT)  rmbHeld = true;
        if (event.button.button == SDL_BUTTON_MIDDLE) mmbHeld = true;
    }
    else if (event.type == SDL_MOUSEBUTTONUP)
    {
        if (event.button.button == SDL_BUTTON_LEFT)   lmbHeld = false;
        if (event.button.button == SDL_BUTTON_RIGHT)  rmbHeld = false;
        if (event.button.button == SDL_BUTTON_MIDDLE) mmbHeld = false;
    }

    // ------------------------------------------------------------------
    // Joystick buttons — event-driven.
    // ------------------------------------------------------------------
    else if (event.type == SDL_JOYBUTTONDOWN)
    {
        activeJoystickButtons.insert(static_cast<int>(event.jbutton.button));
    }
    else if (event.type == SDL_JOYBUTTONUP)
    {
        activeJoystickButtons.erase(static_cast<int>(event.jbutton.button));
    }

    // ------------------------------------------------------------------
    // Joystick axes — track trigger axes for LeftTrigger / RightTrigger.
    // On native SDL the triggers come through as axes; on Emscripten they
    // are mapped to buttons 6/7, handled above via SDL_JOYBUTTONDOWN/UP.
    // ------------------------------------------------------------------
    else if (event.type == SDL_JOYAXISMOTION)
    {
#ifndef __EMSCRIPTEN__
        if (event.jaxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
            leftTriggerAxis = event.jaxis.value / 32767.0f;
        else if (event.jaxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
            rightTriggerAxis = event.jaxis.value / 32767.0f;
#endif
    }

    // ------------------------------------------------------------------
    // Window focus lost — release all held keys / buttons so nothing
    // gets stuck when the window regains focus.
    // ------------------------------------------------------------------
    else if (event.type == SDL_WINDOWEVENT)
    {
        if (event.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
        {
            activeKeys.clear();
            lmbHeld = false;
            rmbHeld = false;
            mmbHeld = false;
            activeJoystickButtons.clear();
            leftTriggerAxis  = 0.f;
            rightTriggerAxis = 0.f;
        }
    }

    // ------------------------------------------------------------------
    // Touch
    // ------------------------------------------------------------------
    else if (event.type == SDL_FINGERDOWN) 
    {

        TouchEvent touchAction;
        touchAction.id = event.tfinger.fingerId + 10;
        touchAction.pressed = true;
        touchAction.released = false;

        touchAction.position = vec2(event.tfinger.x, event.tfinger.y) * UiScreenSize;
        touchAction.delta = vec2(event.tfinger.dx, event.tfinger.dy) * UiScreenSize;

        TouchActions[touchAction.id] = touchAction;

        IsScrenTouched = true;


    }
    else if (event.type == SDL_FINGERMOTION) 
    {

        TouchEvent& touchAction = TouchActions[event.tfinger.fingerId + 10];

        touchAction.position = vec2(event.tfinger.x, event.tfinger.y) * UiScreenSize;
        touchAction.delta = vec2(event.tfinger.dx, event.tfinger.dy) * UiScreenSize;

    }
    else if (event.type == SDL_FINGERUP) 
    {

        TouchEvent& touchAction = TouchActions[event.tfinger.fingerId + 10];

        touchAction.released = true;

        touchAction.position = vec2(event.tfinger.x, event.tfinger.y) * UiScreenSize;
        touchAction.delta = vec2(event.tfinger.dx, event.tfinger.dy) * UiScreenSize;

    }
    else if (event.type == SDL_MOUSEWHEEL) 
    {

        if(event.wheel.preciseY == 0 && event.wheel.y != 0)
        {
                    MouseScrollDelta += event.wheel.y;
        }
        else
        {
            MouseScrollDelta += event.wheel.preciseY;
        }

    }
}

bool Input::IsTouchEventPressed(int id)
{

    if (EngineMain::MainInstance->SimulatingGameTicks) return false;

    auto event = GetTouchEventFromId(id);

    if (event.id == 0) return false;

    return event.pressed;

}

bool Input::IsTouchEventReleased(int id)
{

    if (EngineMain::MainInstance->SimulatingGameTicks) return false;

    auto event = GetTouchEventFromId(id);

    if (event.id == 0) return false;

    return event.released;
}

bool Input::IsTouchEventHolding(int id)
{

    if (EngineMain::MainInstance->SimulatingGameTicks) return false;

	auto event = GetTouchEventFromId(id);

    if (event.id == 1)
    {
        if (GetAction("click")->Holding() == false)
        {
            return false;
        }
    }

    return event.id != 0 && event.released == false;

}

vec2 Input::GetTouchEventPosition(int id)
{

    if (EngineMain::MainInstance->SimulatingGameTicks) return vec2(0);

	auto event = GetTouchEventFromId(id);

	if (event.id == 0) return vec2(0);

	return event.position;

}

vec2 Input::GetTouchEventDelta(int id)
{

    auto event = GetTouchEventFromId(id);

    if (event.id == 0) return vec2(0);

    return event.delta;
}

vec2 Input::GetLeftStickPosition()
{

    if (EngineMain::MainInstance->SimulatingGameTicks) return vec2(0);

    if (joystick)
    {
        const int axisRightX = 0;
        const int axisRightY = 1;

        // Get normalized axis values (range -1 to 1).
        float axisX = SDL_JoystickGetAxis(joystick, axisRightX) / 32768.f;
        float axisY = SDL_JoystickGetAxis(joystick, axisRightY) / -32768.f;

        if (abs(axisX) < 0.1f)
            axisX = 0;

        if (abs(axisY) < 0.1f)
            axisY = 0;

        return vec2(axisX, axisY);

    }

    return vec2();
}

vec2 Input::GetRightStickPosition()
{

    if (EngineMain::MainInstance->SimulatingGameTicks) return vec2(0);

    if (joystick)
    {
        const int axisRightX = 2;
        const int axisRightY = 3;

        // Get normalized axis values (range -1 to 1).
        float axisX = SDL_JoystickGetAxis(joystick, axisRightX) / 32768.f;
        float axisY = SDL_JoystickGetAxis(joystick, axisRightY) / -32768.f;

        if (abs(axisX) < 0.1f)
            axisX = 0;

        if (abs(axisY) < 0.1f)
            axisY = 0;

        return vec2(axisX, axisY);

    }

    return vec2();
}

void Input::UpdateActions() {
    for (auto& pair : actions) {
        pair.second->Update();
    }
}

InputAction* Input::GetAction(const std::string& actionName) {
    if (actions.find(actionName) == actions.end())
        return &InputAction::NullAction;
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

void Input::ReleaseAllActions()
{
    MouseDelta = vec2(0);

    // Clear all event-driven state so nothing stays stuck.
    activeKeys.clear();
    lmbHeld = false;
    rmbHeld = false;
    mmbHeld = false;
    activeJoystickButtons.clear();
    leftTriggerAxis  = 0.f;
    rightTriggerAxis = 0.f;

    for (auto action : actions)
    {
        action.second->CleanInput();
    }
}

void Input::RemoveAction(const std::string& actionName)
{
    InputAction* action = GetAction(actionName);

    if (action == nullptr)
        return;

    actions.erase(actionName);
    delete(action);
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

InputAction* InputAction::AddKeyboardKey(SDL_KeyCode key)
{
    return AddKeyboardKey(SDL_GetScancodeFromKey(key));
}

InputAction* InputAction::RemoveKeyboardKey(SDL_KeyCode key)
{
    return RemoveKeyboardKey(SDL_GetScancodeFromKey(key));
}

InputAction* InputAction::AddButton(GamepadButton button) {
    buttons.push_back(button);
    return this;
}

InputAction* InputAction::RemoveButton(GamepadButton button) {
    buttons.erase(std::remove(buttons.begin(), buttons.end(), button), buttons.end());
    return this;
}

void InputAction::SimulatePressed()
{
    pressed = true;
    pressing = true;
    released = false;
}

void InputAction::SimulateHolding()
{
    pressing = true;
    released = false;
}

void InputAction::SimulateRelease()
{
    released = true;
    pressing = false;
    pressed = false;
}

void InputAction::CleanPressed()
{
    pressed = false;
}

bool InputAction::Pressed() const {
    return pressed;
}

bool InputAction::Released() const {
    return released;
}

bool InputAction::Holding() const {
    return pressing;
}

bool InputAction::PressedBuffered(float bufferLength) const {
    return pressedTime + bufferLength >= Time::GameTimeNoPause;
}

float InputAction::GetHoldTime()
{
    return Time::GameTimeNoPause - pressedTime;
}

void InputAction::Update() {
    bool oldPressing = pressing;
    pressed  = false;
    released = false;
    pressing = false;

    // ------------------------------------------------------------------
    // Mouse buttons — read from event-driven state, not SDL_GetMouseState.
    // ------------------------------------------------------------------
    bool newLmb = Input::lmbHeld;
    bool newRmb = Input::rmbHeld;
    bool newMmb = Input::mmbHeld;

    if (Input::IsScrenTouched)
    {
        newLmb = false;
        newRmb = false;
        newMmb = false;
    }

    if (LMB && newLmb) pressing = true;
    if (RMB && newRmb) pressing = true;
    if (MMB && newMmb) pressing = true;

    // ------------------------------------------------------------------
    // Keyboard — read from event-driven activeKeys set.
    // Key-repeat spam never reaches here: activeKeys is only updated on
    // the first physical KEYDOWN (repeat == 0) and cleared on KEYUP.
    // ------------------------------------------------------------------
    for (auto key : keys)
    {
        if (Input::activeKeys.count(key))
            pressing = true;
    }

    // ------------------------------------------------------------------
    // Joystick buttons — read from event-driven activeJoystickButtons set.
    // ------------------------------------------------------------------
    if (Input::joystick)
    {
        for (auto button : buttons)
        {
            if (button == GamepadButton::LeftTrigger)
            {
                // Native: track via axis value accumulated from SDL_JOYAXISMOTION.
                if (Input::leftTriggerAxis > 0.3f)
                    pressing = true;

#ifdef __EMSCRIPTEN__
                // Emscripten: triggers are mapped to buttons 6/7.
                if (Input::activeJoystickButtons.count(6))
                    pressing = true;
#endif
                continue;
            }

            if (button == GamepadButton::RightTrigger)
            {
                if (Input::rightTriggerAxis > 0.3f)
                    pressing = true;

#ifdef __EMSCRIPTEN__
                if (Input::activeJoystickButtons.count(7))
                    pressing = true;
#endif
                continue;
            }

            if (Input::activeJoystickButtons.count(static_cast<int>(button)))
                pressing = true;
        }
    }

    // Derive pressed / released from the transition.
    if (pressing && !oldPressing) {
        pressed = true;
        pressedTime = Time::GameTimeNoPause;
    }
    else if (!pressing && oldPressing) {
        released = true;
    }
}

void InputAction::CleanInput()
{
    pressed   = false;
    pressing  = false;
    released  = false;
    pressedTime = -100000;
}

TouchEvent Input::GetTouchEventFromId(int id)
{
    auto event = TouchActions.find(id);

    if (event != TouchActions.end())
    {
        return event->second;
    }

    return TouchEvent();

}

int Input::IsAnyTouchEventPressedInBounds(vec2 min, vec2 max)
{
    for (auto& event : TouchActions)
    {

        if (event.second.pressed == false) continue;

        vec2 pos = event.second.position;

        if (pos.x >= min.x && pos.x <= max.x &&
            pos.y >= min.y && pos.y <= max.y)
        {
            return event.first;
        }

    }
    return 0;
}

int Input::IsAnyTouchReleasedPressedInBounds(vec2 min, vec2 max)
{
    for (auto& event : TouchActions)
    {

        if (event.second.released == false) continue;

        vec2 pos = event.second.position;

        if (pos.x >= min.x && pos.x <= max.x &&
            pos.y >= min.y && pos.y <= max.y)
        {
            return event.first;
        }

    }
    return 0;
}

int Input::IsAnyTouchHoldingPressedInBounds(vec2 min, vec2 max)
{
    for (auto& event : TouchActions)
    {

        vec2 pos = event.second.position;

        if (pos.x >= min.x && pos.x <= max.x &&
            pos.y >= min.y && pos.y <= max.y)
        {
            return event.first;
        }

    }
    return 0;
}
