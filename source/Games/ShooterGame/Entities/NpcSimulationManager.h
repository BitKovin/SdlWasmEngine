#pragma once

#include <Entity.h>
#include <map>

struct SchedulePoint
{
	std::string location = "";
	std::string task = "";
};

struct NpcSimulationData
{
	std::string id = "";
	std::string className = "npc_civilian";
	std::map<int, SchedulePoint> schedule;
};

struct NpcSimulationState
{
	std::string id = "";
	std::string currentLocation = "";
	std::string currentTask = "";

	json entityState = json();

};

struct WorldSimulationState
{
	std::map<std::string, NpcSimulationState> npcStates;
};

class NpcSimulationManager : public Entity
{
public:
	NpcSimulationManager();

	void Finalize() override;

	void UpdateDebugUI() override;

	void Start() override;

	static NpcSimulationState* GetSimulationStateRef(const std::string& id);

private:

	static inline int time = 0;

	int startDelay = 2;

	int timeToMinutes(const std::string& timeStr);

	const SchedulePoint* GetActiveSchedulePoint(
		const std::map<int, SchedulePoint>& schedule,
		int currentTimeMinutes);

	void UpdateNpc(NpcSimulationState& npcState);
	void UpdateNpcState(NpcSimulationState& npcState);

	static inline bool LoadedSchedule = false;

	static inline map<std::string, NpcSimulationData> staticSimulationData;

	static inline WorldSimulationState worldSimulationState;


};

