#include "NpcSimulationManager.h"
#include <FileSystem/FileSystem.h>


NpcSimulationManager::NpcSimulationManager()
{

	if (LoadedSchedule) return;

	auto files = FileSystemEngine::GetFilesInPath("GameData/tables/npc/");

	for (auto& file : files)
	{
		Logger::Log(file);
	}

	//lazycsv::parser parse = lazycsv::parser();


}

REGISTER_ENTITY(NpcSimulationManager, "npcSimulationManager")