#include "GameSettings.h"

#include <fstream>
#include <sstream>
#include <Logger.hpp>

std::string GameSettings::DefaultSavePath = "Save/GameSettings.cfg";

GameSettings& GameSettings::Instance()
{
    static GameSettings instance;
    return instance;
}

void GameSettings::ApplyAll() const
{
    Video.ApplyToEngine();
    Audio.ApplyToEngine();
    Input.ApplyToEngine();
}

void GameSettings::ResetAllToDefaults()
{
    Video.ResetToDefaults();
    Audio.ResetToDefaults();
    Input.ResetToDefaults();
}

std::string GameSettings::Serialize() const
{
    std::ostringstream ss;

    ss << "[Video]\n" << Video.Serialize() << "\n";
    ss << "[Audio]\n" << Audio.Serialize() << "\n";
    ss << "[Input.Sensitivity]\n" << Input.SerializeSensitivity() << "\n";
    ss << "[Input.Bindings]\n" << Input.SerializeBindings() << "\n";

    return ss.str();
}

void GameSettings::Deserialize(const std::string& text)
{
    std::istringstream stream(text);
    std::string line;
    std::string section;

    while (std::getline(stream, line))
    {
        // Trim trailing \r (Windows-authored files) and surrounding whitespace.
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
            line.pop_back();

        size_t start = line.find_first_not_of(" \t");
        if (start == std::string::npos) continue; // blank line
        size_t end = line.find_last_not_of(" \t");
        line = line.substr(start, end - start + 1);

        if (line.empty() || line[0] == '#') continue;

        if (line.front() == '[' && line.back() == ']')
        {
            section = line.substr(1, line.size() - 2);
            continue;
        }

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = line.substr(0, eq);
        std::string value = line.substr(eq + 1);

        if (section == "Video")               Video.ApplyLine(key, value);
        else if (section == "Audio")          Audio.ApplyLine(key, value);
        else if (section == "Input.Sensitivity") Input.ApplySensitivityLine(key, value);
        else if (section == "Input.Bindings")    Input.ApplyBindingsLine(key, value);
    }
}

bool GameSettings::SaveToFile(const std::string& path) const
{
    std::ofstream file(path, std::ios::out | std::ios::trunc);
    if (!file.is_open())
    {
        Logger::Log("GameSettings::SaveToFile: could not open '" + path + "' for writing.");
        return false;
    }

    file << Serialize();
    return true;
}

bool GameSettings::LoadFromFile(const std::string& path)
{
    std::ifstream file(path, std::ios::in);
    if (!file.is_open())
        return false; // not an error — first run, or player never saved custom settings

    std::ostringstream ss;
    ss << file.rdbuf();
    Deserialize(ss.str());
    return true;
}
