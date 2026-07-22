#include "InputActionRegistry.h"
#include <Input.h>
#include <Logger.hpp>
#include <algorithm>

std::unordered_map<std::string, InputActionInfo> InputActionRegistry::s_actions;
std::vector<std::string> InputActionRegistry::s_order;
std::vector<std::string> InputActionRegistry::s_categories;

InputActionDefaultBinding InputActionRegistry::CaptureLiveBindingAsDefault(const std::string& action)
{
    InputActionDefaultBinding out;

    InputAction* live = Input::AddAction(action); // creates it if the game hasn't yet — starts empty

    // ── Keyboard / mouse slots ──────────────────────────────────────────────
    int kbSlot = 0;
    for (SDL_Scancode key : live->keys)
    {
        if (kbSlot >= 2)
        {
            Logger::Log("InputActionRegistry: action '" + action +
                        "' has more than 2 default keyboard keys; extra ones are ignored by the rebind UI.");
            break;
        }
        out.kb[kbSlot++] = InputBindingSlotKB::FromKey(key);
    }
    if (kbSlot < 2)
    {
        for (uint8_t mouseButton : live->mouseButtons)
        {
            if (kbSlot >= 2)
            {
                Logger::Log("InputActionRegistry: action '" + action +
                            "' has more than 2 default keyboard/mouse bindings; extra ones are ignored by the rebind UI.");
                break;
            }
            out.kb[kbSlot++] = InputBindingSlotKB::FromMouseButton(mouseButton);
        }
    }

    // ── Gamepad slots ────────────────────────────────────────────────────────
    int gpSlot = 0;
    for (GamepadButton btn : live->buttons)
    {
        if (gpSlot >= 2)
        {
            Logger::Log("InputActionRegistry: action '" + action +
                        "' has more than 2 default gamepad buttons; extra ones are ignored by the rebind UI.");
            break;
        }
        out.gp[gpSlot++] = InputBindingSlotGP::FromButton(btn);
    }

    return out;
}

InputActionInfo* InputActionRegistry::Register(const std::string& action,
                                                 const std::string& displayName,
                                                 const std::string& category)
{
    auto it = s_actions.find(action);
    if (it != s_actions.end())
        return &it->second;

    InputActionInfo info;
    info.name = action;
    info.displayName = displayName;
    info.category = category;
    info.defaults = CaptureLiveBindingAsDefault(action);

    auto inserted = s_actions.emplace(action, info);
    s_order.push_back(action);

    if (std::find(s_categories.begin(), s_categories.end(), category) == s_categories.end())
        s_categories.push_back(category);

    return &inserted.first->second;
}

InputActionInfo* InputActionRegistry::Get(const std::string& action)
{
    auto it = s_actions.find(action);
    return it != s_actions.end() ? &it->second : nullptr;
}

const std::vector<std::string>& InputActionRegistry::GetOrderedActions()
{
    return s_order;
}

const std::vector<std::string>& InputActionRegistry::GetOrderedCategories()
{
    return s_categories;
}

std::vector<std::string> InputActionRegistry::GetActionsInCategory(const std::string& category)
{
    std::vector<std::string> result;
    for (const auto& name : s_order)
    {
        auto it = s_actions.find(name);
        if (it != s_actions.end() && it->second.category == category)
            result.push_back(name);
    }
    return result;
}

void InputActionRegistry::Clear()
{
    s_actions.clear();
    s_order.clear();
    s_categories.clear();
}

// ---------------------------------------------------------------------------
// Example bootstrap — edit the action list to match your game, then call
// this once at startup (after default keys/buttons are bound, before
// GameSettings::Instance().LoadFromFile(...)).
// ---------------------------------------------------------------------------
void RegisterDefaultInputActions()
{
    Input::AddAction("move_forward")->AddKeyboardKey(SDL_SCANCODE_W)->AddButton(GamepadButton::DPadUp);
    Input::AddAction("move_back")->AddKeyboardKey(SDL_SCANCODE_S)->AddButton(GamepadButton::DPadDown);
    Input::AddAction("move_left")->AddKeyboardKey(SDL_SCANCODE_A)->AddButton(GamepadButton::DPadLeft);
    Input::AddAction("move_right")->AddKeyboardKey(SDL_SCANCODE_D)->AddButton(GamepadButton::DPadRight);
    Input::AddAction("jump")->AddKeyboardKey(SDL_SCANCODE_SPACE)->AddButton(GamepadButton::A);
    Input::AddAction("sprint")->AddKeyboardKey(SDL_SCANCODE_LSHIFT)->AddButton(GamepadButton::LeftStick);
    Input::AddAction("interact")->AddKeyboardKey(SDL_SCANCODE_E)->AddButton(GamepadButton::X);
    Input::GetAction("interact"); // (already created above; illustrative)
    Input::AddAction("fire")->AddButton(GamepadButton::RightTrigger)->AddMouseButton(MouseButton::Left);
    Input::AddAction("aim")->AddButton(GamepadButton::LeftTrigger)->AddMouseButton(MouseButton::Right);
    Input::AddAction("reload")->AddKeyboardKey(SDL_SCANCODE_R)->AddButton(GamepadButton::X);
    Input::AddAction("pause")->AddKeyboardKey(SDL_SCANCODE_ESCAPE)->AddButton(GamepadButton::Start);

    InputActionRegistry::Register("move_forward", "Move Forward", "Movement");
    InputActionRegistry::Register("move_back",    "Move Back",    "Movement");
    InputActionRegistry::Register("move_left",    "Move Left",    "Movement");
    InputActionRegistry::Register("move_right",   "Move Right",   "Movement");
    InputActionRegistry::Register("jump",         "Jump",         "Movement");
    InputActionRegistry::Register("sprint",       "Sprint",       "Movement");

    InputActionRegistry::Register("fire",         "Fire",         "Combat");
    InputActionRegistry::Register("aim",          "Aim",          "Combat");
    InputActionRegistry::Register("reload",       "Reload",       "Combat");

    InputActionRegistry::Register("interact",     "Interact",     "General");
    InputActionRegistry::Register("pause",        "Pause",        "General");
}
