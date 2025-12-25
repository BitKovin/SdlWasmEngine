#pragma once

#include <string>
#include "../json.hpp"

using namespace nlohmann;

struct EntitySaveData
{
	std::string className = "";
	std::string id = "";
	json data = json();

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(EntitySaveData, className, id, data)

};