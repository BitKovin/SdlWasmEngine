#include "ConsoleDefaultCommands.h"

#include <vector>	

#include "Console.h"
#include "ConsoleRegister.h"
#include "../Helpers/StringHelper.h"
#include "../Level.hpp"

#include "../Entity.h"
#include "../Physics.h"
#include "../Camera.h"

void CMD_Help(const std::vector<std::string>& args)
{
	Console::Get().PrintCommands(Console::ArgString(args, 0));
}

void CMD_Clear(const std::vector<std::string>& args)
{
	Console::Get().ClearLog();
	Console::Get().AddLog("Console cleared");
}

void CMD_LoadLevel(const std::vector<std::string>& args)
{
	std::string levelName = Console::ArgString(args, 0, "");
	if (levelName != "")
	{
		Level::LoadLevelFromFile("GameData/maps/" + levelName + (StringHelper::EndsWith(levelName, ".bsp") ? "" : ".bsp"));
		Console::Get().AddLog("Loading level: %s", levelName.c_str());
	}
	else
	{
		Console::Get().AddLog("Usage: level.load <level_name>");
	}
}

void CMD_SpawnEntity(const std::vector<std::string>& args)
{
	std::string entityType = Console::ArgString(args, 0, "");
	if (entityType != "")
	{
		// Implementation for spawning entity goes here
		Console::Get().AddLog("Spawning entity of type: %s", entityType.c_str());

		vec3 spawnPoint = Camera::position + Camera::Forward() * 10.0f;

		auto hit = Physics::CylinderTrace(Camera::position, spawnPoint, 0.5f, 0.7f, BodyType::World);

		if (hit.hasHit)
		{
			spawnPoint = hit.shapePosition;
		}

		Entity* entity = Entity::Spawn(entityType);
		if (entity)
		{
			entity->Position = spawnPoint;
			entity->LoadAssetsIfNeeded();
			entity->Start();
		}


	}
	else
	{
		Console::Get().AddLog("Usage: spawn <entity_type>");
	}
}

void ConsoleDefaultCommands::RegisterAll()
{

	REGISTER_CONSOLE_CMD("help", "Displays all commands", CMD_Help);
	REGISTER_CONSOLE_CMD("level.load", "Loads BSP level. Adds .bsp", CMD_LoadLevel);
	REGISTER_CONSOLE_CMD("spawn", "Spawns an entity of specified type", CMD_SpawnEntity);

}
