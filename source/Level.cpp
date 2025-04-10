#include "Level.hpp"

#include "MapData.h"
#include "MapParser.h"

Level* Level::Current = nullptr;

Level* Level::OpenLevel(string filePath)
{
	if (Current)
	{
		Current->Dispose();
		delete(Current);
	}

	Level* newLevel = new Level();

	Current = newLevel;

	MapData mapData = MapParser::ParseMap(filePath);

	mapData.LoadToLevel();

	for (LevelObject* obj : Current->LevelObjects)
	{
		obj->Start();
	}

	printf("generating nav mesh");

	NavigationSystem::GenerateNavData();

	printf("generated nav mesh");

	return newLevel;
}
