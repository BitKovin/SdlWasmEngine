#pragma once

// ---------------------------------------------------------------------------
// AudioSettingsData
//
// Placeholder module, same shape as VideoSettingsData/InputSettingsData, so
// GameSettings has a third nested module to prove out the "serialize the
// whole tree" pattern. ApplyToEngine() is intentionally a stub — wire it up
// to your real audio system (no audio engine headers were provided).
// ---------------------------------------------------------------------------

#include <string>
#include <sstream>
#include <algorithm>

struct AudioSettingsData
{
    float MasterVolume = 1.0f;
    float MusicVolume  = 0.8f;
    float SfxVolume    = 1.0f;

    void ResetToDefaults() { *this = AudioSettingsData(); }

    void ApplyToEngine() const
    {
        // TODO: wire up to the project's audio system, e.g.:
        //   AudioManager::SetMasterVolume(MasterVolume);
        //   AudioManager::SetMusicVolume(MusicVolume);
        //   AudioManager::SetSfxVolume(SfxVolume);
    }

    std::string Serialize() const
    {
        std::ostringstream ss;
        ss << "MasterVolume=" << MasterVolume << "\n";
        ss << "MusicVolume=" << MusicVolume << "\n";
        ss << "SfxVolume=" << SfxVolume << "\n";
        return ss.str();
    }

    void ApplyLine(const std::string& key, const std::string& value)
    {
        try
        {
            float v = std::stof(value);
            v = std::clamp(v, 0.0f, 1.0f);
            if (key == "MasterVolume")     MasterVolume = v;
            else if (key == "MusicVolume") MusicVolume = v;
            else if (key == "SfxVolume")   SfxVolume = v;
        }
        catch (...) { /* malformed line — keep current value */ }
    }
};
