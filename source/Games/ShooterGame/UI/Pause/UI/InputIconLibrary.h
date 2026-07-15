#pragma once

// ---------------------------------------------------------------------------
// InputIconLibrary
//
// Resolves a binding slot to an explicit texture path from the UI icons pack.
// Base path: GameData/textures/ui/icons/
//
// Unlike previous iterations that used generic slugs (e.g. "face_down") and
// required identical filenames per platform folder, this library now maps
// directly to the unique art pack filenames (e.g., "P5Gamepad/T_P5_Cross_Color.png" 
// vs "XGamepad/T_X_A_Color.png").
//
// Gamepad mappings maintain physical button positions. For example, 
// GamepadButton::A represents the "bottom" face button (Xbox A, PS Cross, Switch B).
//
// This file only calls Input.h APIs plus raw SDL2 (SDL_GameController*),
// since gamepad *type* detection isn't exposed by Input.h/Input.cpp today.
// ---------------------------------------------------------------------------

#include <SDL2/SDL.h>
#include <string>
#include <algorithm>
#include <cctype>

#include <Settings/InputBindingTypes.h>

#if defined(SDL_VERSION_ATLEAST)
#define INPUTICON_HAS_GAMECONTROLLER_TYPE SDL_VERSION_ATLEAST(2, 0, 12)
#else
#define INPUTICON_HAS_GAMECONTROLLER_TYPE 0
#endif

enum class GamepadIconPlatform
{
    Xbox,
    PS4,
    PS5,
    Switch
};

class InputIconLibrary
{
public:
    static constexpr const char* BasePath = "GameData/textures/ui/inputs/";
    static constexpr const char* MissingIcon = "GameData/textures/generic/white.png";

    // ── Public entry points used by the UI ──────────────────────────────────
    static std::string GetKeyboardIcon(const InputBindingSlotKB& slot)
    {
        if (!slot.IsBound()) return MissingIcon;

        std::string iconPath;
        switch (slot.kind)
        {
        case KBSlotKind::Mouse: iconPath = GetMouseIconName(slot.mouseButton); break;
        case KBSlotKind::Key:   iconPath = GetKeyIconName(slot.key); break;
        default:                return MissingIcon;
        }

        return iconPath.empty() ? MissingIcon : std::string(BasePath) + iconPath;
    }

    static std::string GetGamepadIcon(const InputBindingSlotGP& slot)
    {
        if (!slot.IsBound()) return MissingIcon;

        std::string iconPath = GetGamepadIconName(DetectConnectedPlatform(), slot.button);
        return iconPath.empty() ? MissingIcon : std::string(BasePath) + iconPath;
    }

private:
    // 0/1/2 get their conventional names; side buttons (3+) fall back to simple mouse
    static std::string GetMouseIconName(uint8_t button)
    {
        switch (button)
        {
        case MouseButton::Left:   return "Keyboard_Mouse/T_Mouse_Left_Key.png";
        case MouseButton::Right:  return "Keyboard_Mouse/T_Mouse_Right_Key.png";
        case MouseButton::Middle: return "Keyboard_Mouse/T_Mouse_Middle_Key.png";
        default:                  return "Keyboard_Mouse/T_Mouse_Simple_Key.png";
        }
    }

    // Detects the first connected SDL game controller's icon family.
    // Falls back to Xbox-style glyphs when no controller is connected or the
    // installed SDL2 predates SDL_GameControllerTypeForIndex (2.0.12).
    static GamepadIconPlatform DetectConnectedPlatform()
    {
#if INPUTICON_HAS_GAMECONTROLLER_TYPE
        int numJoysticks = SDL_NumJoysticks();
        for (int i = 0; i < numJoysticks; ++i)
        {
            if (!SDL_IsGameController(i)) continue;

            SDL_GameControllerType type = SDL_GameControllerTypeForIndex(i);
            switch (type)
            {
            case SDL_CONTROLLER_TYPE_PS3:
            case SDL_CONTROLLER_TYPE_PS4:
                return GamepadIconPlatform::PS4;
#if SDL_VERSION_ATLEAST(2, 0, 14)
            case SDL_CONTROLLER_TYPE_PS5:
                return GamepadIconPlatform::PS5;
            case SDL_CONTROLLER_TYPE_NINTENDO_SWITCH_PRO:
                return GamepadIconPlatform::Switch;
#endif
            default:
                return GamepadIconPlatform::Xbox;
            }
        }
#endif
        return GamepadIconPlatform::Xbox;
    }

    static std::string GetGamepadIconName(GamepadIconPlatform platform, GamepadButton button)
    {
        switch (platform)
        {
        case GamepadIconPlatform::PS4:    return GetPS4IconName(button);
        case GamepadIconPlatform::PS5:    return GetPS5IconName(button);
        case GamepadIconPlatform::Switch: return GetSwitchIconName(button);
        case GamepadIconPlatform::Xbox:
        default:                          return GetXboxIconName(button);
        }
    }

    // Physical Mapping: A=Bottom, B=Right, X=Left, Y=Top
    static std::string GetXboxIconName(GamepadButton button)
    {
        switch (button)
        {
        case GamepadButton::A:             return "XGamepad/T_X_A_Color.png";
        case GamepadButton::B:             return "XGamepad/T_X_B_Color.png";
        case GamepadButton::X:             return "XGamepad/T_X_X_Color.png";
        case GamepadButton::Y:             return "XGamepad/T_X_Y_Color.png";
        case GamepadButton::DPadUp:        return "XGamepad/T_X_Dpad_Up.png";
        case GamepadButton::DPadDown:      return "XGamepad/T_X_Dpad_Down.png";
        case GamepadButton::DPadLeft:      return "XGamepad/T_X_Dpad_Left.png";
        case GamepadButton::DPadRight:     return "XGamepad/T_X_Dpad_Right.png";
        case GamepadButton::LeftShoulder:  return "XGamepad/T_X_LB.png";
        case GamepadButton::RightShoulder: return "XGamepad/T_X_RB.png";
        case GamepadButton::LeftTrigger:   return "XGamepad/T_X_LT.png";
        case GamepadButton::RightTrigger:  return "XGamepad/T_X_RT.png";
        case GamepadButton::LeftStick:     return "XGamepad/T_X_Left_Stick_Click.png";
        case GamepadButton::RightStick:    return "XGamepad/T_X_Right_Stick_Click.png";
        case GamepadButton::Back:          return "XGamepad/T_X_Share.png"; // Closest equivalent in pack
        default:                           return "";
        }
    }

    // Physical Mapping: A=Cross, B=Circle, X=Square, Y=Triangle
    static std::string GetPS4IconName(GamepadButton button)
    {
        switch (button)
        {
        case GamepadButton::A:             return "P4Gamepad/T_P4_Cross_Color.png";
        case GamepadButton::B:             return "P4Gamepad/T_P4_Circle_Color.png";
        case GamepadButton::X:             return "P4Gamepad/T_P4_Square_Color.png";
        case GamepadButton::Y:             return "P4Gamepad/T_P4_Triangle_Color.png";
        case GamepadButton::DPadUp:        return "P4Gamepad/T_P4_Dpad_UP.png";
        case GamepadButton::DPadDown:      return "P4Gamepad/T_P4_Dpad_Down.png";
        case GamepadButton::DPadLeft:      return "P4Gamepad/T_P4_Dpad_Left.png";
        case GamepadButton::DPadRight:     return "P4Gamepad/T_P4_Dpad_Right.png";
        case GamepadButton::LeftShoulder:  return "P4Gamepad/T_P4_L1.png";
        case GamepadButton::RightShoulder: return "P4Gamepad/T_P4_R1.png";
        case GamepadButton::LeftTrigger:   return "P4Gamepad/T_P4_L2.png";
        case GamepadButton::RightTrigger:  return "P4Gamepad/T_P4_R2.png";
        case GamepadButton::LeftStick:     return "P4Gamepad/T_P4_L3.png";
        case GamepadButton::RightStick:    return "P4Gamepad/T_P4_R3.png";
        case GamepadButton::Start:         return "P4Gamepad/T_P4_Options.png";
        case GamepadButton::Back:          return "P4Gamepad/T_P4_Share.png";
        case GamepadButton::Touchpad:      return "P4Gamepad/T_P4_Touch_Pad.png";
        default:                           return "";
        }
    }

    // Physical Mapping: A=Cross, B=Circle, X=Square, Y=Triangle
    static std::string GetPS5IconName(GamepadButton button)
    {
        switch (button)
        {
        case GamepadButton::A:             return "P5Gamepad/T_P5_Cross_Color.png";
        case GamepadButton::B:             return "P5Gamepad/T_P5_Circle_Color.png";
        case GamepadButton::X:             return "P5Gamepad/T_P5_Square_Color.png";
        case GamepadButton::Y:             return "P5Gamepad/T_P5_Triangle_Color.png";
        case GamepadButton::DPadUp:        return "P5Gamepad/T_P5_Dpad_UP.png";
        case GamepadButton::DPadDown:      return "P5Gamepad/T_P5_Dpad_Down.png";
        case GamepadButton::DPadLeft:      return "P5Gamepad/T_P5_Dpad_Left.png";
        case GamepadButton::DPadRight:     return "P5Gamepad/T_P5_Dpad_Right.png";
        case GamepadButton::LeftShoulder:  return "P5Gamepad/T_P5_L1.png";
        case GamepadButton::RightShoulder: return "P5Gamepad/T_P5_R1.png";
        case GamepadButton::LeftTrigger:   return "P5Gamepad/T_P5_L2.png";
        case GamepadButton::RightTrigger:  return "P5Gamepad/T_P5_R2.png";
        case GamepadButton::LeftStick:     return "P5Gamepad/T_P5_L3.png";
        case GamepadButton::RightStick:    return "P5Gamepad/T_P5_R3.png";
        case GamepadButton::Start:         return "P5Gamepad/T_P5_Options.png";
        case GamepadButton::Back:          return "P5Gamepad/T_P5_Share.png";
        case GamepadButton::Touchpad:      return "P5Gamepad/T_P5_Touch_Pad.png";
        default:                           return "";
        }
    }

    // Physical Mapping: A=B, B=A, X=Y, Y=X
    static std::string GetSwitchIconName(GamepadButton button)
    {
        switch (button)
        {
        case GamepadButton::A:             return "SGamepad/T_S_B.png";
        case GamepadButton::B:             return "SGamepad/T_S_A.png";
        case GamepadButton::X:             return "SGamepad/T_S_Y.png";
        case GamepadButton::Y:             return "SGamepad/T_S_X.png";
        case GamepadButton::DPadUp:        return "SGamepad/T_S_Dpad_Up.png";
        case GamepadButton::DPadDown:      return "SGamepad/T_S_Dpad_Down.png";
        case GamepadButton::DPadLeft:      return "SGamepad/T_S_Dpad_Left.png";
        case GamepadButton::DPadRight:     return "SGamepad/T_S_Dpad_Right.png";
        case GamepadButton::LeftShoulder:  return "SGamepad/T_S_LB.png";
        case GamepadButton::RightShoulder: return "SGamepad/T_S_RB.png";
        case GamepadButton::LeftTrigger:   return "SGamepad/T_S_LT.png";
        case GamepadButton::RightTrigger:  return "SGamepad/T_S_RT.png";
        case GamepadButton::LeftStick:     return "SGamepad/T_S_L.png"; // Fallback to stick icon
        case GamepadButton::RightStick:    return "SGamepad/T_S_R.png"; // Fallback to stick icon
        case GamepadButton::Start:         return "SGamepad/T_S_Plus.png";
        case GamepadButton::Back:          return "SGamepad/T_S_Minus.png";
        case GamepadButton::Guide:         return "SGamepad/T_S_Home.png";
        default:                           return "";
        }
    }

    // Maps explicit keys, handling known typos in the art pack (like T_Crtl_Key.png)
    static std::string GetKeyIconName(SDL_Scancode key)
    {
        std::string prefix = "Keyboard_Mouse/T_";
        std::string suffix = "_Key.png";

        switch (key)
        {
        case SDL_SCANCODE_SPACE:     return prefix + "Space" + suffix;
        case SDL_SCANCODE_RETURN:    return prefix + "Enter" + suffix;
        case SDL_SCANCODE_ESCAPE:    return prefix + "Esc" + suffix;
        case SDL_SCANCODE_BACKSPACE: return prefix + "BackSpace" + suffix;
        case SDL_SCANCODE_TAB:       return prefix + "Tab" + suffix;
        case SDL_SCANCODE_LSHIFT:
        case SDL_SCANCODE_RSHIFT:    return prefix + "Shift" + suffix;
        case SDL_SCANCODE_LCTRL:
        case SDL_SCANCODE_RCTRL:     return prefix + "Crtl" + suffix; // Artpack typo match
        case SDL_SCANCODE_LALT:
        case SDL_SCANCODE_RALT:      return prefix + "Alt" + suffix;
        case SDL_SCANCODE_CAPSLOCK:  return prefix + "CapsLock" + suffix;
        case SDL_SCANCODE_UP:        return prefix + "Up" + suffix;
        case SDL_SCANCODE_DOWN:      return prefix + "Down" + suffix;
        case SDL_SCANCODE_LEFT:      return prefix + "Left" + suffix;
        case SDL_SCANCODE_RIGHT:     return prefix + "Right" + suffix;
        case SDL_SCANCODE_DELETE:    return prefix + "Del" + suffix;
        case SDL_SCANCODE_INSERT:    return prefix + "Ins" + suffix;
        case SDL_SCANCODE_HOME:      return prefix + "Home" + suffix;
        case SDL_SCANCODE_END:       return prefix + "End" + suffix;
        case SDL_SCANCODE_PAGEUP:    return prefix + "PageUp" + suffix;
        case SDL_SCANCODE_PAGEDOWN:  return prefix + "PageDown" + suffix;
        case SDL_SCANCODE_GRAVE:     return prefix + "Tilde" + suffix;
        case SDL_SCANCODE_MINUS:     return prefix + "Minus" + suffix;
        case SDL_SCANCODE_EQUALS:    return prefix + "Plus" + suffix;
        case SDL_SCANCODE_LEFTBRACKET: return prefix + "Brackets_L" + suffix;
        case SDL_SCANCODE_RIGHTBRACKET:return prefix + "Brackets_R" + suffix;
        case SDL_SCANCODE_SEMICOLON: return prefix + "Semicolon" + suffix;
        case SDL_SCANCODE_APOSTROPHE:return prefix + "Quotation" + suffix;
        case SDL_SCANCODE_SLASH:     return prefix + "Slash" + suffix;
        default: break;
        }

        // Handle F1-F12
        if (key >= SDL_SCANCODE_F1 && key <= SDL_SCANCODE_F12)
            return prefix + "F" + std::to_string(1 + (key - SDL_SCANCODE_F1)) + suffix;

        // Handle A-Z
        if (key >= SDL_SCANCODE_A && key <= SDL_SCANCODE_Z)
        {
            std::string letter(1, static_cast<char>('A' + (key - SDL_SCANCODE_A)));
            return prefix + letter + suffix;
        }

        // Handle 1-9
        if (key >= SDL_SCANCODE_1 && key <= SDL_SCANCODE_9)
            return prefix + std::to_string(1 + (key - SDL_SCANCODE_1)) + suffix;

        // Handle 0
        if (key == SDL_SCANCODE_0)
            return prefix + "0" + suffix;

        // Fallback for unmapped keys (causes GetKeyboardIcon to return MissingIcon)
        return "";
    }
};