#include "LevelTraversalSystem.h"
#include "Entity.h"
#include "json.hpp"
#include "SaveSystem/LevelSaveSystem.h"

void LevelTraversalSystem::TravelToLevel(std::string LevelPath, Entity* playerEntity, std::string desiredSpawnPointName)
{

	Traveling = true;
	bool oldSaveGame = playerEntity->SaveGame;

	playerEntity->SaveGame = false;

	LevelSaveData currentLevelData = LevelSaveSystem::SaveLevelToData();

	LevelMemory[currentLevelData.name] = currentLevelData;

	playerEntity->SaveGame = oldSaveGame;

	json j;
	playerEntity->Serialize(j);
	PlayerSerializedData = j;
	DesiredSpawnPointName = desiredSpawnPointName;

	LevelSaveData levelData = LevelSaveData();

	if (LevelMemory.count(LevelPath))
	{

		levelData = LevelMemory[LevelPath];
	}
	
	LevelSaveSystem::pendingSave = levelData;

	float timeToSimulate = Time::GameTime - levelData.GameTime;

	TimeSkip = timeToSimulate;

	Level::LoadLevelFromFile(LevelPath);

}

void LevelTraversalSystem::FinishTransition()
{

	PlayerSerializedData = json();
	DesiredSpawnPointName = "";
	Traveling = false;

}

void LevelTraversalSystem::Reset()
{

	PlayerSerializedData = json();
	DesiredSpawnPointName = "";
	LevelMemory = {};
	TimeSkip = 0;
}
