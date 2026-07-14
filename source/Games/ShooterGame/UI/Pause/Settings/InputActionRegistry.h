#pragma once

// ---------------------------------------------------------------------------
// InputActionRegistry
//
// The code-side, NOT-serialized half of the input settings system.
//
//   1. At startup, after your game has set up each InputAction with its
//      default keys/buttons (Input::AddAction("jump")->AddKeyboardKey(...)),
//      call InputActionRegistry::Register(...) once per action the player
//      should be able to see/rebind in the settings menu. Register()
//      snapshots whatever is currently bound as that action's DEFAULT.
//
//   2. InputSettingsData (the serialized, saveable half) only ever stores
//      OVERRIDES on top of these defaults — see InputSettingsData.h.
//
// Actions you never Register() here simply don't show up in the rebind UI
// and can't be overridden by it (useful for purely internal actions like
// "click" or "ui_confirm").
//
// See RegisterDefaultInputActions() near the bottom for a template to copy
// into your own game-specific bootstrap code.
// ---------------------------------------------------------------------------

#include <string>
#include <vector>
#include <unordered_map>

#include "InputBindingTypes.h"

struct InputActionDefaultBinding
{
    InputBindingSlotKB kb[2];
    InputBindingSlotGP gp[2];
};

struct InputActionInfo
{
    std::string name;         // matches the key used with Input::AddAction/GetAction
    std::string displayName;  // shown in the UI, e.g. "Move Forward"
    std::string category;     // row grouping, e.g. "Movement"
    InputActionDefaultBinding defaults;
};

class InputActionRegistry
{
public:
    // Idempotent: calling twice with the same action name returns the
    // existing entry without re-capturing defaults.
    static InputActionInfo* Register(const std::string& action,
                                      const std::string& displayName,
                                      const std::string& category = "General");

    static InputActionInfo* Get(const std::string& action);

    static const std::vector<std::string>& GetOrderedActions();
    static const std::vector<std::string>& GetOrderedCategories();

    // Actions belonging to a given category, in registration order.
    static std::vector<std::string> GetActionsInCategory(const std::string& category);

    static void Clear(); // mainly for tests / hot-reload scenarios

private:
    static std::unordered_map<std::string, InputActionInfo> s_actions;
    static std::vector<std::string> s_order;
    static std::vector<std::string> s_categories;

    static InputActionDefaultBinding CaptureLiveBindingAsDefault(const std::string& action);
};

// ---------------------------------------------------------------------------
// Template bootstrap. NOT called automatically — copy/adapt this into your
// game's startup sequence (after default InputActions are configured, before
// GameSettings::Instance().LoadFromFile(...) is called so overrides have
// defaults to apply on top of).
// ---------------------------------------------------------------------------
void RegisterDefaultInputActions();
