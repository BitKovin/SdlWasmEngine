#pragma once

// ---------------------------------------------------------------------------
// InputSettingsData
//
// The serialized, saveable half of the input settings system.
//
//   Sensitivity           — plain values, always fully serialized.
//   BindingOverrides       — SPARSE map<action, override>. An action only
//                             appears here once the player has changed at
//                             least one of its slots away from the default
//                             registered in InputActionRegistry. Actions the
//                             player never touched are simply absent, and
//                             GetEffectiveBinding() transparently falls back
//                             to InputActionRegistry's default for them.
//
// ApplyToEngine() is the only place that reaches into Input::actions and
// mutates live InputAction state — call it once after loading, and again any
// time a rebind commits.
// ---------------------------------------------------------------------------

#include <string>
#include <unordered_map>

#include "InputBindingTypes.h"
#include "InputActionRegistry.h"

struct InputBindingOverride
{
    InputBindingSlotKB kb[2];
    InputBindingSlotGP gp[2];

    bool operator==(const InputBindingOverride& other) const
    {
        return kb[0] == other.kb[0] && kb[1] == other.kb[1] &&
               gp[0] == other.gp[0] && gp[1] == other.gp[1];
    }

    static InputBindingOverride FromDefault(const InputActionDefaultBinding& d)
    {
        InputBindingOverride o;
        o.kb[0] = d.kb[0]; o.kb[1] = d.kb[1];
        o.gp[0] = d.gp[0]; o.gp[1] = d.gp[1];
        return o;
    }
};

struct InputSensitivitySettings
{
    float MouseSensitivity     = 0.22f; // mirrors Input::sensitivity's engine default
    float GamepadLookSensitivity = 1.0f; // multiplier; read by your camera code
    bool  InvertY               = false; // read by your camera code

    static constexpr float MinMouseSensitivity = 0.01f;
    static constexpr float MaxMouseSensitivity = 2.0f;
    static constexpr float MinGamepadSensitivity = 0.1f;
    static constexpr float MaxGamepadSensitivity = 3.0f;

    void ResetToDefaults();
    void ApplyToEngine() const; // pushes MouseSensitivity -> Input::sensitivity

    std::string Serialize() const;                                   // body of [Input.Sensitivity]
    void ApplyLine(const std::string& key, const std::string& value); // one key=value from that section
};

class InputSettingsData
{
public:
    InputSensitivitySettings Sensitivity;
    std::unordered_map<std::string, InputBindingOverride> BindingOverrides;

    void ResetSensitivityToDefaults();
    void ResetBindingToDefault(const std::string& action);
    void ResetAllBindingsToDefaults();
    void ResetToDefaults(); // sensitivity + all bindings

    // Pushes Sensitivity + the effective (default+override) binding of every
    // registered action into the live Input system.
    void ApplyToEngine() const;

    // Effective binding = override slot if present, else the registry default.
    InputBindingOverride GetEffectiveBinding(const std::string& action) const;

    // ── Mutators used by the rebind UI ─────────────────────────────────────
    // Returns the name of a conflicting action if `value` is already bound
    // elsewhere (and allowConflict is false) — the UI should confirm with the
    // player and retry with allowConflict=true, which also clears the slot
    // that was conflicting. Returns "" on success.
    std::string SetKeyboardSlot(const std::string& action, int slotIndex, InputBindingSlotKB value, bool allowConflict = false);
    std::string SetGamepadSlot(const std::string& action, int slotIndex, InputBindingSlotGP value, bool allowConflict = false);

    void ClearKeyboardSlot(const std::string& action, int slotIndex);
    void ClearGamepadSlot(const std::string& action, int slotIndex);

    // ── Serialization ────────────────────────────────────────────────────────
    std::string SerializeSensitivity() const; // body of [Input.Sensitivity]
    std::string SerializeBindings() const;    // body of [Input.Bindings]

    void ApplySensitivityLine(const std::string& key, const std::string& value);
    void ApplyBindingsLine(const std::string& key, const std::string& value); // key=action, value="kb1|kb2|gp1|gp2"

private:
    InputBindingOverride& EditableOverride(const std::string& action);
    void PruneIfMatchesDefault(const std::string& action);

    std::string FindKeyboardConflict(const std::string& action, int slotIndex, const InputBindingSlotKB& value) const;
    std::string FindGamepadConflict(const std::string& action, int slotIndex, const InputBindingSlotGP& value) const;
};
