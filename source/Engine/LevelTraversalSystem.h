#pragma once

#include <string>
#include <unordered_map>
#include "SaveSystem/LevelSaveData.h"
#include "json.hpp"

class Entity;

class LevelTraversalSystem
{
public:
	
	static inline json PlayerSerializedData = "";
	static inline std::string DesiredSpawnPointName = "";

	static inline float TimeSkip = 0;

	static inline std::unordered_map<std::string, LevelSaveData> LevelMemory;

	static void TravelToLevel(std::string LevelPath, Entity* playerEntity, std::string desiredSpawnPointName);

	static inline bool Traveling = false;

	static void FinishTransition();
	static void Reset();
private:


};

