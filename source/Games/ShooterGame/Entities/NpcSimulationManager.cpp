#include "NpcSimulationManager.h"
#include <FileSystem/FileSystem.h>
#include <Helpers/CSVParser.h>
#include <sstream>
#include "Npc/NpcBase.h"

NpcSimulationManager::NpcSimulationManager()
{

	if (LoadedSchedule)
	{

		Start();

		return;
	}

	staticSimulationData = {};

	auto files = FileSystemEngine::GetFilesInPath("GameData/tables/npc/");

	for (auto& file : files)
	{

		std::map<int, SchedulePoint> schedule;

		std::string text = FileSystemEngine::ReadFile("GameData/tables/npc/" + file);

		text.erase(0, text.find_first_not_of("\r\n"));
		text.erase(text.find_last_not_of("\r\n") + 1);

		CSVParser parser = CSVParser(text);

		for (size_t row = 1; row < parser.rowCount(); ++row) {
			std::string time = parser.at(row,0).trimmed();  // column 0
			std::string state = parser.at(row, 1).trimmed();  // column 1

			if (!time.empty()) 
			{  

				auto stateArr = StringHelper::ParseCommaSeparatedStrings(state);

				std::string location = stateArr[0];
				std::string task = stateArr[1];
				int timeM = timeToMinutes(time);

				SchedulePoint p;
				p.location = location;
				p.task = task;

				schedule[timeM] = p;

			}
		}

		std::string classname = parser.at(parser.findCell("classname") + ivec2(0,1));

		NpcSimulationData npcSimData;
		npcSimData.schedule = schedule;
		npcSimData.className = classname;

		std::filesystem::path p(file);
		npcSimData.id = p.stem().string();		 

		staticSimulationData[npcSimData.id] = npcSimData;

	}

	Start();

}

void NpcSimulationManager::Finalize()
{

	if (startDelay > 0)
	{
		startDelay--;
		return;
	}

	for (auto& npcState : worldSimulationState.npcStates)
	{

		if (npcState.second.dead) continue;

		UpdateNpcState(npcState.second);
		UpdateNpc(npcState.second);

	}

}

void NpcSimulationManager::UpdateDebugUI()
{

	ImGui::Begin("simulation");

	ImGui::DragInt("time", &time);

	ImGui::End();

}

void NpcSimulationManager::Start()
{

	for (auto& staticNpcData : staticSimulationData)
	{
		if (worldSimulationState.npcStates.count(staticNpcData.first))
		{

		}
		else
		{

			NpcSimulationState npcState;
			npcState.id = staticNpcData.second.id;

			worldSimulationState.npcStates[npcState.id] = npcState;
		}
	}

}

NpcSimulationState* NpcSimulationManager::GetSimulationStateRef(const std::string& id)
{
	auto res = worldSimulationState.npcStates.find(id);

	if (res != worldSimulationState.npcStates.end())
	{
		return &res->second;
	}

	return nullptr;
}

void NpcSimulationManager::UpdateNpc(NpcSimulationState& npcState)
{

	std::string& map = Level::Current->mapName;

	NpcBase* npcRef = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithName(npcState.id));

	bool exists = npcRef != nullptr;

	bool shouldExist = npcState.currentLocation == map;


	if (exists == false && shouldExist)
	{
		npcRef = dynamic_cast<NpcBase*>(LevelObjectFactory::instance().create(staticSimulationData[npcState.id].className));
		npcRef->Name = npcState.id;
		npcRef->SimulationId = npcState.id;
		Level::Current->AddEntity(npcRef);
		npcRef->LoadAssetsIfNeeded();
		npcRef->Start();
		npcRef->Deserialize(npcState.entityState);


		auto task = Level::Current->FindEntityWithName(npcState.currentTask);

		auto hit = Physics::LineTrace(task->Position + vec3(0,1,0), task->Position - vec3(0, 2, 0), BodyType::World);

		if (hit.hasHit)
		{
			npcRef->Teleport(hit.position + vec3(0, 1, 0));
		}
		else
		{
			npcRef->Teleport(task->Position + vec3(0, 0.2f, 0));
		}



		exists = true;

	}

	if (exists)
	{



		if (shouldExist == false)
		{
			json j;
			npcRef->Serialize(j);
			npcState.entityState = j;
			npcRef->Destroy();
			npcRef = nullptr;
			exists = false;
		}

	}

}

void NpcSimulationManager::UpdateNpcState(NpcSimulationState& npcState)
{

	auto& simData = staticSimulationData[npcState.id];

	auto schedulePoint = GetActiveSchedulePoint(simData.schedule, time);

	npcState.currentLocation = schedulePoint->location;
	npcState.currentTask = schedulePoint->task;

}

int NpcSimulationManager::timeToMinutes(const std::string& timeStr)
{
	int hours = 0, minutes = 0;
	char colon;

	std::istringstream iss(timeStr);
	if (!(iss >> hours >> colon >> minutes) || colon != ':' ||
		hours < 0 || hours > 23 || minutes < 0 || minutes > 59) {
		return -1;  // Invalid
	}

	return hours * 60 + minutes;
}

const SchedulePoint* NpcSimulationManager::GetActiveSchedulePoint(const std::map<int, SchedulePoint>& schedule, int currentTimeMinutes)
{
	auto it = schedule.upper_bound(currentTimeMinutes);

	if (it == schedule.begin())
	{

		it = schedule.upper_bound(10000000000000);//try end
		if (it == schedule.begin())
		{
			return nullptr;
		}
	}

	--it;
	return &it->second;
}

REGISTER_ENTITY(NpcSimulationManager, "npcSimulationManager")