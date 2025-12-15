#include "ConsoleDefaultCommands.h"

#include <vector>	

#include "Console.h"
#include "ConsoleRegister.h"
#include "../Helpers/StringHelper.h"
#include "../Level.hpp"

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

void ConsoleDefaultCommands::RegisterAll()
{

	REGISTER_CONSOLE_CMD("help", "Displays all commands", CMD_Help);
	REGISTER_CONSOLE_CMD("level.load", "Loads BSP level. Adds .bsp", CMD_LoadLevel);

}
