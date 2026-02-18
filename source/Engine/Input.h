#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_video.h>
#include "glm.h"

#include "Logger.hpp"

// Forward declaration for InputAction.
class InputAction;

// Base class for a custom mouse movement calculator (override GetMouseDelta if desired)
class MouseMoveCalculator {
public:
    virtual glm::vec2 GetMouseDelta() { return glm::vec2(); }
};

#ifndef __EMSCRIPTEN__

enum class GamepadButton : int {
    Invalid = SDL_CONTROLLER_BUTTON_INVALID,   // usually -1
    A = SDL_CONTROLLER_BUTTON_A,         // 0
    B = SDL_CONTROLLER_BUTTON_B,         // 1
    X = SDL_CONTROLLER_BUTTON_X,         // 2
    Y = SDL_CONTROLLER_BUTTON_Y,         // 3
    Back = SDL_CONTROLLER_BUTTON_BACK,      // 4
    Guide = SDL_CONTROLLER_BUTTON_GUIDE,     // 5
    Start = SDL_CONTROLLER_BUTTON_START,     // 6
    LeftStick = SDL_CONTROLLER_BUTTON_LEFTSTICK, // 7
    RightStick = SDL_CONTROLLER_BUTTON_RIGHTSTICK,// 8
    LeftShoulder = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,  // 9
    RightShoulder = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, // 10
    DPadUp = SDL_CONTROLLER_BUTTON_DPAD_UP,       // 11
    DPadDown = SDL_CONTROLLER_BUTTON_DPAD_DOWN,     // 12
    DPadLeft = SDL_CONTROLLER_BUTTON_DPAD_LEFT,     // 13
    DPadRight = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,    // 14
    Misc1 = SDL_CONTROLLER_BUTTON_MISC1,     // 15, platform‑specific
    Paddle1 = SDL_CONTROLLER_BUTTON_PADDLE1,   // 16
    Paddle2 = SDL_CONTROLLER_BUTTON_PADDLE2,   // 17
    Paddle3 = SDL_CONTROLLER_BUTTON_PADDLE3,   // 18
    Paddle4 = SDL_CONTROLLER_BUTTON_PADDLE4,   // 19
    Touchpad = SDL_CONTROLLER_BUTTON_TOUCHPAD,  // 20
    Max = SDL_CONTROLLER_BUTTON_MAX,       // 21

    LeftTrigger = 101,
    RightTrigger = 102

};

#else


enum class GamepadButton : int {
    Invalid = SDL_CONTROLLER_BUTTON_INVALID,   // usually -1
    A = 0,         // 0
    B = 1,         // 1
    X = 3,         // 2
    Y = 2,         // 3
    Back = 8,      // 4
    Guide = 16,     // 5
    Start = 9,     // 6
    LeftStick = 10, // 7
    RightStick = 11,// 8
    LeftShoulder = 4,  // 9
    RightShoulder = 5, // 10
    DPadUp = 12,       // 11
    DPadDown = 13,     // 12
    DPadLeft = 14,     // 13
    DPadRight = 15,    // 14
    Misc1 = SDL_CONTROLLER_BUTTON_MISC1,     // 15, platform‑specific
    Paddle1 = SDL_CONTROLLER_BUTTON_PADDLE1,   // 16
    Paddle2 = SDL_CONTROLLER_BUTTON_PADDLE2,   // 17
    Paddle3 = SDL_CONTROLLER_BUTTON_PADDLE3,   // 18
    Paddle4 = SDL_CONTROLLER_BUTTON_PADDLE4,   // 19
    Touchpad = SDL_CONTROLLER_BUTTON_TOUCHPAD,  // 20
    Max = SDL_CONTROLLER_BUTTON_MAX,       // 21

    LeftTrigger = 101,
    RightTrigger = 102

};

#endif

struct TouchEvent
{
    int id = 0;

    bool pressed = false;
    bool released = false;

    vec2 position = vec2();
    vec2 delta = vec2();

    TouchEvent()
    {

    }

};

class Input 
{
public:
    // Mouse tracking
    static glm::vec2 MousePos;
    static glm::vec2 MouseDelta;
    static glm::vec2 PendingMouseDelta;
    static std::vector<glm::vec2> MouseDeltas;
    static int MaxDeltas;

    // Input actions mapped by a string name
    static std::unordered_map<std::string, InputAction*> actions;

    // Cursor locking and sensitivity
    static bool LockCursor;
    static float sensitivity;
    static glm::vec2 windowCenter;
    static bool PendingCenterCursor;

    static bool PendingWindowStateReset;

    static std::unordered_map<int, TouchEvent> TouchActions;

    static bool IsScrenTouched;

    // Optional custom mouse movement calculator
    static MouseMoveCalculator* mouseMoveCalculator;

    // SDL window pointer (set in main)
    static SDL_Window* window;
    // SDL_Joystick pointer (if a joystick is connected)
    static SDL_Joystick* joystick;

    // Must be called every frame to update input state.
    static void Update();

    // Apply joystick (right thumbstick) input to MouseDelta.
    static void JoystickCamera();

    // Update mouse state.
    static void UpdateMouse();

    static void UpdateMousePosition();

    // Adds the mouse delta to a rolling average.
    static void AddMouseInput(const glm::vec2& delta);

    // Centers the mouse cursor to the window center.
    static void CenterCursor();

    static void StartEventsFrame();
    static void ReceiveSdlEvent(SDL_Event event);

    //to do
    static bool IsTouchEventPressed(int id);
    static bool IsTouchEventReleased(int id);
    static bool IsTouchEventHolding(int id);
    static vec2 GetTouchEventPosition(int id);
    static vec2 GetTouchEventDelta(int id);

    static TouchEvent GetTouchEventFromId(int id);

    static int IsAnyTouchEventPressedInBounds(vec2 min, vec2 max);
    static int IsAnyTouchReleasedPressedInBounds(vec2 min, vec2 max);
    static int IsAnyTouchHoldingPressedInBounds(vec2 min, vec2 max);

    //static TouchEvent* GetTouchEventFromId(int id);

    static vec2 GetLeftStickPosition();

    static vec2 GetRightStickPosition();

    // Update all registered input actions.
    static void UpdateActions();

    // Retrieve an action by name (or nullptr if not found).
    static InputAction* GetAction(const std::string& actionName);

    // Add a new input action (or return the existing one if already added).
    static InputAction* AddAction(const std::string& actionName);

    static void RemoveAction(const std::string& actionName);

    static void ReleaseAllActions();

};

class InputAction 
{
public:
    std::vector<SDL_Scancode> keys;
    // For joystick buttons, we simply use int indices.
    std::vector<GamepadButton> buttons;
    bool LMB = false;
    bool RMB = false;
    bool MMB = false;

    double pressedTime = 0.0;

    InputAction();

    InputAction* AddKeyboardKey(SDL_Scancode key);
    InputAction* RemoveKeyboardKey(SDL_Scancode key);

    InputAction* AddKeyboardKey(SDL_KeyCode key);
    InputAction* RemoveKeyboardKey(SDL_KeyCode key);

    InputAction* AddButton(GamepadButton button);
    InputAction* RemoveButton(GamepadButton button);

    void SimulatePressed();
    void SimulateHolding();
    void SimulateRelease();

    // Returns true on the frame the action was pressed.
    bool Pressed() const;
    // Returns true on the frame the action was released.
    bool Released() const;
    // Returns true while the action is being held.
    bool Holding() const;
    // Returns true if the action was pressed within the given buffer time.
    bool PressedBuffered(float bufferLength = 0.2f) const;

    float GetHoldTime();

    // Call every frame to update the action state.
    void Update();

    void CleanInput();

    static InputAction NullAction;

protected:

    bool pressing = false;
    bool released = false;
    bool pressed = false;

    void OnDispose()
    {
        Logger::Log("destroying input action");
    }
};
