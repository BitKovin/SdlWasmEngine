#ifndef INPUT_H
#define INPUT_H

#include <SDL/SDL.h>
#include <SDL/SDL_joystick.h>
#include <unordered_map>
#include <vector>
#include <string>
#include <SDL/SDL_scancode.h>
#include <SDL/SDL_video.h>

#include "EObject.hpp"
#include "Vector2.h"
#include "Logger.hpp"

// Forward declaration for InputAction.
class InputAction;

// Base class for a custom mouse movement calculator (override GetMouseDelta if desired)
class MouseMoveCalculator {
public:
    virtual Vector2 GetMouseDelta() { return Vector2(); }
};

class Input {
public:
    // Mouse tracking
    static Vector2 MousePos;
    static Vector2 MouseDelta;
    static std::vector<Vector2> MouseDeltas;
    static int MaxDeltas;

    // Input actions mapped by a string name
    static std::unordered_map<std::string, InputAction*> actions;

    // Cursor locking and sensitivity
    static bool LockCursor;
    static float sensitivity;
    static Vector2 windowCenter;
    static bool PendingCenterCursor;

    // Optional custom mouse movement calculator
    static MouseMoveCalculator* mouseMoveCalculator;

    // SDL window pointer (set in main)
    static SDL_Window* window;
    // SDL_Joystick pointer (if a joystick is connected)
    static SDL_Joystick* joystick;

    // Timing
    static Uint32 lastTime;   // Last frame time in ms.
    static float deltaTime;   // Delta time in seconds.
    static double GameTime;   // Game time in seconds.

    // Must be called every frame to update input state.
    static void Update();

    // Apply joystick (right thumbstick) input to MouseDelta.
    static void JoystickCamera();

    // Update mouse state.
    static void UpdateMouse();

    // Adds the mouse delta to a rolling average.
    static void AddMouseInput(const Vector2& delta);

    // Centers the mouse cursor to the window center.
    static void CenterCursor();

    // Update all registered input actions.
    static void UpdateActions();

    // Retrieve an action by name (or nullptr if not found).
    static InputAction* GetAction(const std::string& actionName);

    // Add a new input action (or return the existing one if already added).
    static InputAction* AddAction(const std::string& actionName);

    static void RemoveAction(const std::string& actionName);
};

class InputAction : public EObject {
public:
    std::vector<SDL_Scancode> keys;
    // For joystick buttons, we simply use int indices.
    std::vector<int> buttons;
    bool LMB = false;
    bool RMB = false;
    bool MMB = false;

    bool pressing = false;
    bool released = false;
    bool pressed = false;

    double pressedTime = 0.0;

    InputAction();

    InputAction* AddKeyboardKey(SDL_Scancode key);
    InputAction* RemoveKeyboardKey(SDL_Scancode key);
    InputAction* AddButton(int button);
    InputAction* RemoveButton(int button);

    // Returns true on the frame the action was pressed.
    bool Pressed();
    // Returns true on the frame the action was released.
    bool Released();
    // Returns true while the action is being held.
    bool Holding();
    // Returns true if the action was pressed within the given buffer time.
    bool PressedBuffered(float bufferLength = 0.2f);

    // Call every frame to update the action state.
    void Update();

protected:

    void OnDispose()
    {
        Logger::Log("destroying input action");
    }


};


#endif // INPUT_H
