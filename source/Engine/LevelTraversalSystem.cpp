#include "LevelTraversalSystem.h"
#include "Entity.h"
#include "json.hpp"
#include "SaveSystem/LevelSaveSystem.h"

void LevelTraversalSystem::TravelToLevel(std::string LevelPath, Entity* playerEntity, std::string desiredSpawnPointName)
{


	bool oldSaveGame = playerEntity->SaveGame;

	playerEntity->SaveGame = false;

	LevelSaveData currentLevelData = LevelSaveSystem::SaveLevelToData();

	LevelMemory[currentLevelData.name] = currentLevelData;

	playerEntity->SaveGame = oldSaveGame;

	json j;
	playerEntity->Serialize(j);
	PlayerSerializedData = j;
	DesiredSpawnPointName = desiredSpawnPointName;

	LevelSaveData levelData;

	if (LevelMemory.count(LevelPath))
	{

		levelData = LevelMemory[LevelPath];
	}
	
	LevelSaveSystem::pendingSave = levelData;

	Level::LoadLevelFromFile(LevelPath);

}
