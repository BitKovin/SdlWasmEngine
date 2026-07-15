#include "LevelTraversalSystem.h"
#include "Entity.h"
#include "json.hpp"
#include "SaveSystem/LevelSaveSystem.h"

void LevelTraversalSystem::TravelToLevel(std::string LevelPath, Entity* playerEntity, std::string desiredSpawnPointName, std::string levelIdentifier)
{

	Traveling = true;
	bool oldSaveGame = playerEntity->SaveGame;

	if (levelIdentifier.empty())
		levelIdentifier = LevelPath;

	playerEntity->SaveGame = false;

	LevelSaveData currentLevelData = LevelSaveSystem::SaveLevelToData();

	std::string currentIdentifier = currentLevelData.identifier.empty() ? currentLevelData.name : currentLevelData.identifier;

	LevelMemory[currentIdentifier] = currentLevelData;

	playerEntity->SaveGame = oldSaveGame;

	json j;
	playerEntity->Serialize(j);
	PlayerSerializedData = j;
	DesiredSpawnPointName = desiredSpawnPointName;

	LevelSaveData levelData = LevelSaveData();

	if (LevelMemory.count(levelIdentifier))
	{
		levelData = LevelMemory[levelIdentifier];
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
