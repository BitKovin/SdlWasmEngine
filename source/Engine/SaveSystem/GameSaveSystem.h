#pragma once
#include "LevelSaveSystem.h"
#include "../LevelTraversalSystem.h"

struct GameSaveData
{
	LevelSaveData LevelData;
	std::unordered_map<std::string, LevelSaveData> LevelMemory;
	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(GameSaveData, LevelData, LevelMemory)
};

class GameSaveSystem
{
public:
	
	static GameSaveData SaveGameToData();
	static void LoadGameFromData(GameSaveData data);

	static void SaveGameToFile(std::string saveName);
	static void LoadGameFromFile(std::string saveName);

	static inline bool LoadingGame = false;

private:

	static inline string saveDataPath = "SaveData/";

};

