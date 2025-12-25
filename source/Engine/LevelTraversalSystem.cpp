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
	
	float timeToSimulate = Time::GameTime - levelData.GameTime;

	float simulationLimit = 100; // in future I should replace it with time since start of the day or smth

	if (timeToSimulate > simulationLimit) //limiting simulation time and snappting game time to only simulate remaining time
	{
		timeToSimulate = simulationLimit;

	}

	levelData.GameTime = Time::GameTime - timeToSimulate;

	LevelSaveSystem::pendingSave = levelData;



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
