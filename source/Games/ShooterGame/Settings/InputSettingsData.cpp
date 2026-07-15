#include "InputSettingsData.h"
#include "../Input.h"

#include <sstream>
#include <algorithm>
#include <cctype>

using namespace InputBindingSerialization;

// ── InputSensitivitySettings ────────────────────────────────────────────────

void InputSensitivitySettings::ResetToDefaults()
{
    *this = InputSensitivitySettings();
}

void InputSensitivitySettings::ApplyToEngine() const
{
    Input::sensitivity = std::clamp(MouseSensitivity, MinMouseSensitivity, MaxMouseSensitivity);
    Input::FlipMouseY = InvertY;
}

std::string InputSensitivitySettings::Serialize() const
{
    std::ostringstream ss;
    ss << "MouseSensitivity=" << MouseSensitivity << "\n";
    ss << "GamepadLookSensitivity=" << GamepadLookSensitivity << "\n";
    ss << "InvertY=" << (InvertY ? 1 : 0) << "\n";
    return ss.str();
}

void InputSensitivitySettings::ApplyLine(const std::string& key, const std::string& value)
{
    try
    {
        if (key == "MouseSensitivity")        MouseSensitivity = std::stof(value);
        else if (key == "GamepadLookSensitivity") GamepadLookSensitivity = std::stof(value);
        else if (key == "InvertY")            InvertY = (value == "1" || value == "true");
    }
    catch (...) { /* malformed line — keep current value */ }
}

// ── InputSettingsData ────────────────────────────────────────────────────────

void InputSettingsData::ResetSensitivityToDefaults()
{
    Sensitivity.ResetToDefaults();
}

void InputSettingsData::ResetBindingToDefault(const std::string& action)
{
    BindingOverrides.erase(action);
}

void InputSettingsData::ResetAllBindingsToDefaults()
{
    BindingOverrides.clear();
}

void InputSettingsData::ResetToDefaults()
{
    ResetSensitivityToDefaults();
    ResetAllBindingsToDefaults();
}

InputBindingOverride InputSettingsData::GetEffectiveBinding(const std::string& action) const
{
    auto overrideIt = BindingOverrides.find(action);
    if (overrideIt != BindingOverrides.end())
        return overrideIt->second;

    const InputActionInfo* info = InputActionRegistry::Get(action);
    if (!info)
        return InputBindingOverride{};

    return InputBindingOverride::FromDefault(info->defaults);
}

void InputSettingsData::ApplyToEngine() const
{
    Sensitivity.ApplyToEngine();

    for (const std::string& action : InputActionRegistry::GetOrderedActions())
    {
        InputAction* live = Input::AddAction(action);
        InputBindingOverride eff = GetEffectiveBinding(action);

        live->keys.clear();
        live->buttons.clear();
        live->mouseButtons.clear();

        for (int i = 0; i < 2; ++i)
        {
            switch (eff.kb[i].kind)
            {
            case KBSlotKind::Key:   live->keys.push_back(eff.kb[i].key); break;
            case KBSlotKind::Mouse: live->mouseButtons.push_back(eff.kb[i].mouseButton); break;
            default: break;
            }

            if (eff.gp[i].kind == GPSlotKind::Button)
                live->buttons.push_back(eff.gp[i].button);
        }
    }
}

InputBindingOverride& InputSettingsData::EditableOverride(const std::string& action)
{
    auto it = BindingOverrides.find(action);
    if (it != BindingOverrides.end())
        return it->second;

    // Seed a fresh override from the current effective binding so slots the
    // caller isn't touching right now aren't accidentally cleared.
    InputBindingOverride seeded = GetEffectiveBinding(action);
    auto inserted = BindingOverrides.emplace(action, seeded);
    return inserted.first->second;
}

void InputSettingsData::PruneIfMatchesDefault(const std::string& action)
{
    auto it = BindingOverrides.find(action);
    if (it == BindingOverrides.end()) return;

    const InputActionInfo* info = InputActionRegistry::Get(action);
    if (!info) return;

    InputBindingOverride def = InputBindingOverride::FromDefault(info->defaults);
    if (it->second == def)
        BindingOverrides.erase(it); // back to matching the default — no need to store it
}

std::string InputSettingsData::FindKeyboardConflict(const std::string& action, int slotIndex, const InputBindingSlotKB& value) const
{
    if (!value.IsBound()) return "";

    for (const std::string& other : InputActionRegistry::GetOrderedActions())
    {
        InputBindingOverride eff = GetEffectiveBinding(other);
        for (int i = 0; i < 2; ++i)
        {
            if (other == action && i == slotIndex) continue;
            if (eff.kb[i] == value) return other;
        }
    }
    return "";
}

std::string InputSettingsData::FindGamepadConflict(const std::string& action, int slotIndex, const InputBindingSlotGP& value) const
{
    if (!value.IsBound()) return "";

    for (const std::string& other : InputActionRegistry::GetOrderedActions())
    {
        InputBindingOverride eff = GetEffectiveBinding(other);
        for (int i = 0; i < 2; ++i)
        {
            if (other == action && i == slotIndex) continue;
            if (eff.gp[i] == value) return other;
        }
    }
    return "";
}

std::string InputSettingsData::SetKeyboardSlot(const std::string& action, int slotIndex, InputBindingSlotKB value, bool allowConflict)
{
    slotIndex = std::clamp(slotIndex, 0, 1);

    std::string conflict = FindKeyboardConflict(action, slotIndex, value);
    if (!conflict.empty() && !allowConflict)
        return conflict;

    if (!conflict.empty() && allowConflict)
    {
        InputBindingOverride& conflictOverride = EditableOverride(conflict);
        for (int i = 0; i < 2; ++i)
            if (conflictOverride.kb[i] == value)
                conflictOverride.kb[i] = InputBindingSlotKB::None();
        PruneIfMatchesDefault(conflict);
    }

    InputBindingOverride& ov = EditableOverride(action);
    ov.kb[slotIndex] = value;
    PruneIfMatchesDefault(action);
    return "";
}

std::string InputSettingsData::SetGamepadSlot(const std::string& action, int slotIndex, InputBindingSlotGP value, bool allowConflict)
{
    slotIndex = std::clamp(slotIndex, 0, 1);

    std::string conflict = FindGamepadConflict(action, slotIndex, value);
    if (!conflict.empty() && !allowConflict)
        return conflict;

    if (!conflict.empty() && allowConflict)
    {
        InputBindingOverride& conflictOverride = EditableOverride(conflict);
        for (int i = 0; i < 2; ++i)
            if (conflictOverride.gp[i] == value)
                conflictOverride.gp[i] = InputBindingSlotGP::None();
        PruneIfMatchesDefault(conflict);
    }

    InputBindingOverride& ov = EditableOverride(action);
    ov.gp[slotIndex] = value;
    PruneIfMatchesDefault(action);
    return "";
}

void InputSettingsData::ClearKeyboardSlot(const std::string& action, int slotIndex)
{
    slotIndex = std::clamp(slotIndex, 0, 1);
    InputBindingOverride& ov = EditableOverride(action);
    ov.kb[slotIndex] = InputBindingSlotKB::None();
    PruneIfMatchesDefault(action);
}

void InputSettingsData::ClearGamepadSlot(const std::string& action, int slotIndex)
{
    slotIndex = std::clamp(slotIndex, 0, 1);
    InputBindingOverride& ov = EditableOverride(action);
    ov.gp[slotIndex] = InputBindingSlotGP::None();
    PruneIfMatchesDefault(action);
}

// ── Serialization ────────────────────────────────────────────────────────────

std::string InputSettingsData::SerializeSensitivity() const
{
    return Sensitivity.Serialize();
}

std::string InputSettingsData::SerializeBindings() const
{
    std::ostringstream ss;
    for (const std::string& action : InputActionRegistry::GetOrderedActions())
    {
        auto it = BindingOverrides.find(action);
        if (it == BindingOverrides.end()) continue; // sparse — only true overrides are written

        const InputBindingOverride& ov = it->second;
        ss << action << "="
           << KBToString(ov.kb[0]) << "|"
           << KBToString(ov.kb[1]) << "|"
           << GPToString(ov.gp[0]) << "|"
           << GPToString(ov.gp[1]) << "\n";
    }
    return ss.str();
}

void InputSettingsData::ApplySensitivityLine(const std::string& key, const std::string& value)
{
    Sensitivity.ApplyLine(key, value);
}

void InputSettingsData::ApplyBindingsLine(const std::string& key, const std::string& value)
{
    // value = "kb1|kb2|gp1|gp2", any field may be empty
    std::string fields[4];
    int idx = 0;
    std::string cur;
    for (char c : value)
    {
        if (c == '|')
        {
            if (idx < 4) fields[idx] = cur;
            cur.clear();
            ++idx;
        }
        else cur += c;
    }
    if (idx < 4) fields[idx] = cur;

    InputBindingOverride ov;
    ov.kb[0] = KBFromString(fields[0]);
    ov.kb[1] = KBFromString(fields[1]);
    ov.gp[0] = GPFromString(fields[2]);
    ov.gp[1] = GPFromString(fields[3]);

    BindingOverrides[key] = ov;
    PruneIfMatchesDefault(key); // in case the save file stored a no-op override
}
