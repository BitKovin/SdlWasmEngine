#pragma once

// ---------------------------------------------------------------------------
// InputBindingTypes.h
//
// Small, dependency-light value types shared by:
//   - InputActionRegistry   (code-registered defaults, captured at startup)
//   - InputSettingsData     (runtime overrides, serialized to disk)
//   - InputIconLibrary      (icon lookup)
//   - The rebinding UI      (UiBindSlotButton / UiRebindCaptureModal)
//
// A "keyboard slot" can hold a keyboard scancode OR a mouse button, because
// the shipped icon packs (and most players' mental model) treat keyboard +
// mouse as a single device. A "gamepad slot" holds a GamepadButton (which
// already includes LeftTrigger/RightTrigger as pseudo-buttons, see Input.h).
//
// Every action gets exactly 2 keyboard slots + 2 gamepad slots. This cap is
// enforced here, not in Input.h/InputAction, so the underlying engine input
// system is untouched.
// ---------------------------------------------------------------------------

#include <SDL2/SDL.h>
#include <SDL2/SDL_scancode.h>
#include <string>
#include <algorithm>
#include <cctype>

#include <Input.h>

// ── Keyboard / mouse slot ───────────────────────────────────────────────────

enum class KBSlotKind
{
    None,
    Key,
    Mouse
};

struct InputBindingSlotKB
{
    KBSlotKind   kind = KBSlotKind::None;
    SDL_Scancode key = SDL_SCANCODE_UNKNOWN;
    uint8_t      mouseButton = 0; // valid when kind == Mouse; see MouseButton namespace in Input.h

    bool IsBound() const { return kind != KBSlotKind::None; }

    bool operator==(const InputBindingSlotKB& other) const
    {
        if (kind != other.kind) return false;
        if (kind == KBSlotKind::Key) return key == other.key;
        if (kind == KBSlotKind::Mouse) return mouseButton == other.mouseButton;
        return true;
    }
    bool operator!=(const InputBindingSlotKB& other) const { return !(*this == other); }

    static InputBindingSlotKB None() { return InputBindingSlotKB{}; }
    static InputBindingSlotKB FromKey(SDL_Scancode k) { InputBindingSlotKB s; s.kind = KBSlotKind::Key; s.key = k; return s; }
    static InputBindingSlotKB FromMouseButton(uint8_t button) { InputBindingSlotKB s; s.kind = KBSlotKind::Mouse; s.mouseButton = button; return s; }
};

// ── Gamepad slot ─────────────────────────────────────────────────────────────

enum class GPSlotKind
{
    None,
    Button
};

struct InputBindingSlotGP
{
    GPSlotKind    kind = GPSlotKind::None;
    GamepadButton button = GamepadButton::Invalid;

    bool IsBound() const { return kind != GPSlotKind::None; }

    bool operator==(const InputBindingSlotGP& other) const
    {
        if (kind != other.kind) return false;
        if (kind == GPSlotKind::Button) return button == other.button;
        return true;
    }
    bool operator!=(const InputBindingSlotGP& other) const { return !(*this == other); }

    static InputBindingSlotGP None() { return InputBindingSlotGP{}; }
    static InputBindingSlotGP FromButton(GamepadButton b) { InputBindingSlotGP s; s.kind = GPSlotKind::Button; s.button = b; return s; }
};

// ── String (de)serialization ────────────────────────────────────────────────
// Used for the save file AND for building human-readable fallback labels.
// Deliberately independent of SDL_GetScancodeName so save files stay stable
// across SDL versions/locales.

namespace InputBindingSerialization
{
    inline std::string ToUpper(std::string s)
    {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
        return s;
    }

    // "" | "KEY:<SDL scancode name>" | "MOUSE:<button index>"
    inline std::string KBToString(const InputBindingSlotKB& slot)
    {
        switch (slot.kind)
        {
        case KBSlotKind::Key:
        {
            const char* name = SDL_GetScancodeName(slot.key);
            if (!name || !name[0]) return "";
            return std::string("KEY:") + name;
        }
        case KBSlotKind::Mouse:
            return "MOUSE:" + std::to_string(static_cast<unsigned int>(slot.mouseButton));
        default:
            return "";
        }
    }

    inline InputBindingSlotKB KBFromString(const std::string& s)
    {
        if (s.empty()) return InputBindingSlotKB::None();

        if (s.rfind("KEY:", 0) == 0)
        {
            SDL_Scancode sc = SDL_GetScancodeFromName(s.substr(4).c_str());
            if (sc == SDL_SCANCODE_UNKNOWN) return InputBindingSlotKB::None();
            return InputBindingSlotKB::FromKey(sc);
        }
        if (s.rfind("MOUSE:", 0) == 0)
        {
            try
            {
                int button = std::stoi(s.substr(6));
                if (button < 0 || button > 255) return InputBindingSlotKB::None();
                return InputBindingSlotKB::FromMouseButton(static_cast<uint8_t>(button));
            }
            catch (...) { return InputBindingSlotKB::None(); }
        }
        return InputBindingSlotKB::None();
    }

    // "" | "BUTTON:<EnumName>"
    inline std::string GPButtonEnumName(GamepadButton b)
    {
        switch (b)
        {
        case GamepadButton::A:             return "A";
        case GamepadButton::B:             return "B";
        case GamepadButton::X:             return "X";
        case GamepadButton::Y:             return "Y";
        case GamepadButton::Back:          return "Back";
        case GamepadButton::Guide:         return "Guide";
        case GamepadButton::Start:         return "Start";
        case GamepadButton::LeftStick:     return "LeftStick";
        case GamepadButton::RightStick:    return "RightStick";
        case GamepadButton::LeftShoulder:  return "LeftShoulder";
        case GamepadButton::RightShoulder: return "RightShoulder";
        case GamepadButton::DPadUp:        return "DPadUp";
        case GamepadButton::DPadDown:      return "DPadDown";
        case GamepadButton::DPadLeft:      return "DPadLeft";
        case GamepadButton::DPadRight:     return "DPadRight";
        case GamepadButton::Misc1:         return "Misc1";
        case GamepadButton::Paddle1:       return "Paddle1";
        case GamepadButton::Paddle2:       return "Paddle2";
        case GamepadButton::Paddle3:       return "Paddle3";
        case GamepadButton::Paddle4:       return "Paddle4";
        case GamepadButton::Touchpad:      return "Touchpad";
        case GamepadButton::LeftTrigger:   return "LeftTrigger";
        case GamepadButton::RightTrigger:  return "RightTrigger";
        default:                          return "";
        }
    }

    inline GamepadButton GPButtonFromEnumName(const std::string& name)
    {
        if (name == "A")             return GamepadButton::A;
        if (name == "B")             return GamepadButton::B;
        if (name == "X")             return GamepadButton::X;
        if (name == "Y")             return GamepadButton::Y;
        if (name == "Back")          return GamepadButton::Back;
        if (name == "Guide")         return GamepadButton::Guide;
        if (name == "Start")         return GamepadButton::Start;
        if (name == "LeftStick")     return GamepadButton::LeftStick;
        if (name == "RightStick")    return GamepadButton::RightStick;
        if (name == "LeftShoulder")  return GamepadButton::LeftShoulder;
        if (name == "RightShoulder") return GamepadButton::RightShoulder;
        if (name == "DPadUp")        return GamepadButton::DPadUp;
        if (name == "DPadDown")      return GamepadButton::DPadDown;
        if (name == "DPadLeft")      return GamepadButton::DPadLeft;
        if (name == "DPadRight")     return GamepadButton::DPadRight;
        if (name == "Misc1")         return GamepadButton::Misc1;
        if (name == "Paddle1")       return GamepadButton::Paddle1;
        if (name == "Paddle2")       return GamepadButton::Paddle2;
        if (name == "Paddle3")       return GamepadButton::Paddle3;
        if (name == "Paddle4")       return GamepadButton::Paddle4;
        if (name == "Touchpad")      return GamepadButton::Touchpad;
        if (name == "LeftTrigger")   return GamepadButton::LeftTrigger;
        if (name == "RightTrigger")  return GamepadButton::RightTrigger;
        return GamepadButton::Invalid;
    }

    inline std::string GPToString(const InputBindingSlotGP& slot)
    {
        if (slot.kind != GPSlotKind::Button) return "";
        std::string name = GPButtonEnumName(slot.button);
        if (name.empty()) return "";
        return "BUTTON:" + name;
    }

    inline InputBindingSlotGP GPFromString(const std::string& s)
    {
        if (s.rfind("BUTTON:", 0) != 0) return InputBindingSlotGP::None();
        GamepadButton b = GPButtonFromEnumName(s.substr(7));
        if (b == GamepadButton::Invalid) return InputBindingSlotGP::None();
        return InputBindingSlotGP::FromButton(b);
    }

    // Short, human-readable fallback text shown on a bind-slot button when no
    // icon texture is available (or in addition to it as a caption).
    inline std::string KBDisplayName(const InputBindingSlotKB& slot)
    {
        switch (slot.kind)
        {
        case KBSlotKind::Key:
        {
            const char* name = SDL_GetScancodeName(slot.key);
            return (name && name[0]) ? std::string(name) : std::string("?");
        }
        case KBSlotKind::Mouse:
            switch (slot.mouseButton)
            {
            case MouseButton::Left:   return "Mouse L";
            case MouseButton::Right:  return "Mouse R";
            case MouseButton::Middle: return "Mouse M";
            default:                 return "Mouse " + std::to_string(static_cast<unsigned int>(slot.mouseButton) - MouseButton::Side1 + 1);
            }
        default:
            return "";
        }
    }

    inline std::string GPDisplayName(const InputBindingSlotGP& slot)
    {
        if (slot.kind != GPSlotKind::Button) return "";
        switch (slot.button)
        {
        case GamepadButton::A:             return "A";
        case GamepadButton::B:             return "B";
        case GamepadButton::X:             return "X";
        case GamepadButton::Y:             return "Y";
        case GamepadButton::Back:          return "Back";
        case GamepadButton::Guide:         return "Guide";
        case GamepadButton::Start:         return "Start";
        case GamepadButton::LeftStick:     return "L3";
        case GamepadButton::RightStick:    return "R3";
        case GamepadButton::LeftShoulder:  return "LB";
        case GamepadButton::RightShoulder: return "RB";
        case GamepadButton::DPadUp:        return "D-Up";
        case GamepadButton::DPadDown:      return "D-Down";
        case GamepadButton::DPadLeft:      return "D-Left";
        case GamepadButton::DPadRight:     return "D-Right";
        case GamepadButton::LeftTrigger:   return "LT";
        case GamepadButton::RightTrigger:  return "RT";
        case GamepadButton::Touchpad:      return "Touchpad";
        default:                          return GPButtonEnumName(slot.button);
        }
    }
}
