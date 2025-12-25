#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "../json.hpp"
#include "EntitySaveData.h"

using namespace nlohmann;

struct LevelSaveData
{
	std::string name = "";
	std::vector<EntitySaveData> entities;
	std::unordered_map<std::string, int> nextId;

	std::vector<std::string> deletedNames;
	std::vector<std::string> deletedIDs;

	double GameTime = 0;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(LevelSaveData, name, entities, nextId, deletedNames, deletedIDs, GameTime)

};