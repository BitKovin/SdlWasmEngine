#include "GameSaveSystem.h"
#include "../Level.hpp"

GameSaveData GameSaveSystem::SaveGameToData()
{

    GameSaveData data;

    data.LevelData = LevelSaveSystem::SaveLevelToData();
    data.LevelMemory = LevelTraversalSystem::LevelMemory;

    return data;
}

void GameSaveSystem::LoadGameFromData(GameSaveData data)
{

    LevelTraversalSystem::Reset();

    LevelTraversalSystem::LevelMemory = data.LevelMemory;
    LoadingGame = true;

    LevelSaveSystem::pendingSave = data.LevelData;
    LevelTraversalSystem::Traveling = false;
    Level::LoadLevelFromFile(data.LevelData.name);

}



void GameSaveSystem::SaveGameToFile(std::string saveName)
{

    GameSaveData data = SaveGameToData();

    json j = json(data);

    FileSystemEngine::WriteFile(saveDataPath + saveName + ".savg", j.dump());

}

void GameSaveSystem::LoadGameFromFile(std::string saveName)
{

    string text = FileSystemEngine::ReadFile(saveDataPath + saveName + ".savg");

    if (text.size() < 1)
    {
        Logger::Log("failed to load save" + saveName);
        return;
    }


    json jsonData = json::parse(text);

    GameSaveData gameData = jsonData.get<GameSaveData>();

    LoadGameFromData(gameData);

}

std::vector<std::string> GameSaveSystem::GetAllSaves()
{
    std::vector<std::string> files = FileSystemEngine::GetFilesInPath(saveDataPath);

    for (auto& f : files)
    {
        constexpr std::string_view extension = ".savg";

        if (f.size() >= extension.size() &&
            f.compare(f.size() - extension.size(), extension.size(), extension) == 0)
        {
            f.erase(f.size() - extension.size());
        }
    }

    return files;
}