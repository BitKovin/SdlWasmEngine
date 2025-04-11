#include "Level.hpp"

#include "MapData.h"
#include "MapParser.h"

#include "Physics.h"

Level* Level::Current = nullptr;

void Level::CloseLevel()
{
	for (LevelObject* obj : Current->LevelObjects)
	{
		obj->Dispose();
		delete(obj);
	}
	Current->LevelObjects.clear();

	Physics::DestroyAllBodies();

}

Level* Level::OpenLevel(string filePath)
{
	if (Current)
	{

		CloseLevel();

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
