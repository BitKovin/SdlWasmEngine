#pragma once

#include <Entity.h>

struct NpcSimulationData
{
	std::string className = "npc_civilian";
	std::unordered_map<int, string> schedule;
};

class NpcSimulationManager : public Entity
{
public:
	NpcSimulationManager();

private:

	static inline bool LoadedSchedule = false;

};

