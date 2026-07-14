#pragma once

// ---------------------------------------------------------------------------
// VideoSettingsData
//
// A small PERSISTED settings struct — distinct from the existing
// Settings/VideoSettings.h + VideoSettingsModel, which (per UiVideoSettings.h)
// is a UI-population helper (e.g. "list the resolutions available on this
// display"), not saved state. This struct is the thing that actually gets
// written to disk and re-applied on the next launch.
//
// Kept intentionally small: it exists mainly to demonstrate how GameSettings
// nests independent settings modules that each own their own Apply/Reset/
// Serialize. Extend the fields to match whatever UiVideoSettings.h exposes.
// ---------------------------------------------------------------------------

#include <string>
#include <sstream>

struct VideoSettingsData
{
    int Width = 1920;
    int Height = 1080;
    std::string WindowMode = "windowed"; // "windowed" | "fullscreen" | "borderless"
    bool VSync = true;

    void ResetToDefaults() { *this = VideoSettingsData(); }

    void ApplyToEngine() const; // implemented in VideoSettingsData.cpp

    std::string Serialize() const
    {
        std::ostringstream ss;
        ss << "Width=" << Width << "\n";
        ss << "Height=" << Height << "\n";
        ss << "WindowMode=" << WindowMode << "\n";
        ss << "VSync=" << (VSync ? 1 : 0) << "\n";
        return ss.str();
    }

    void ApplyLine(const std::string& key, const std::string& value)
    {
        try
        {
            if (key == "Width")           Width = std::stoi(value);
            else if (key == "Height")     Height = std::stoi(value);
            else if (key == "WindowMode") WindowMode = value;
            else if (key == "VSync")      VSync = (value == "1" || value == "true");
        }
        catch (...) { /* malformed line — keep current value */ }
    }
};
