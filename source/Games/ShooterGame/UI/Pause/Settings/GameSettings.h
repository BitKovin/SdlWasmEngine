#pragma once

// ---------------------------------------------------------------------------
// GameSettings
//
// Single statically-accessible root for every settings module in the game:
//
//   GameSettings::Instance().Video.Width = 2560;
//   GameSettings::Instance().Input.Sensitivity.MouseSensitivity = 0.4f;
//   GameSettings::Instance().ApplyAll();
//   GameSettings::Instance().SaveToFile();
//
// Any UI screen (video settings, input settings, audio settings, ...) reads
// and writes directly into these nested structs, then calls ApplyAll() (or a
// module's own ApplyToEngine()) to push the change live, and SaveToFile() to
// persist it. Nothing renders or touches engine state on its own just by
// being constructed — these are plain data + apply/reset/serialize methods.
//
// Adding a new settings module (e.g. GraphicsQuality, Accessibility, ...):
//   1. Give it the same shape as VideoSettingsData/AudioSettingsData:
//        ResetToDefaults(), ApplyToEngine(), Serialize(), ApplyLine(key,value)
//   2. Add a field for it here.
//   3. Add its section to GameSettings::Serialize()/Deserialize() below.
// ---------------------------------------------------------------------------

#include <string>

#include "VideoSettingsData.h"
#include "AudioSettingsData.h"
#include "InputSettingsData.h"

class GameSettings
{
public:
    static GameSettings& Instance();

    VideoSettingsData Video;
    AudioSettingsData Audio;
    InputSettingsData Input;

    void ApplyAll() const;
    void ResetAllToDefaults();

    // Default save location; override before calling Save/Load if your
    // project has its own save-directory convention (e.g. SDL_GetPrefPath).
    static std::string DefaultSavePath;

    bool SaveToFile(const std::string& path = DefaultSavePath) const;
    bool LoadFromFile(const std::string& path = DefaultSavePath);

    std::string Serialize() const;
    void Deserialize(const std::string& text);

private:
    GameSettings() = default;
};
