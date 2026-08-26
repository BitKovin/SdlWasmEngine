#pragma once

#pragma once
#include "Console.h"

struct ConsoleCommandRegistrar
{
    ConsoleCommandRegistrar(const ConsoleCommand& cmd)
    {
        Console::Get().RegisterCommand(cmd);
    }
};

#define REGISTER_CONSOLE_COMMAND(NAME, HELP, FUNC) \
    static ConsoleCommandRegistrar _reg_##FUNC({ NAME, HELP, FUNC })

#define REGISTER_CONSOLE_CMD(NAME, HELP, FUNC) \
    Console::Get().RegisterCommand({ NAME, HELP, FUNC })

#define CONCAT_IMPL(x, y) x##y
#define CONCAT(x, y) CONCAT_IMPL(x, y)

#define CONSOLE_FUNC(CMD_NAME, HELP)       \
    static void CONCAT(_cmd_, __LINE__)(const std::vector<std::string>&);   \
    static ConsoleCommandRegistrar CONCAT(_reg_, __LINE__)({                \
        CMD_NAME,                                                   \
        HELP,                                                       \
        CONCAT(_cmd_, __LINE__)                                     \
    });                                                             \
    static void CONCAT(_cmd_, __LINE__)(const std::vector<std::string>& args)