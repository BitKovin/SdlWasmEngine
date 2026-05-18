#include "Logger.hpp"

#include <Console/Console.h>

#include <iostream>

void Logger::LogInternal(Level level, const char* format, va_list args)
{

#ifndef DISTRIBUTION
    char messageBuffer[1024];
    vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);

    std::string timestamp = GetTimestamp();
    const char* levelStr = GetLevelString(level);

    {
        std::lock_guard<std::mutex> lock(g_LogMutex);

        std::cout << std::format("[{}] [{}] {}\n",
                                 timestamp, levelStr, messageBuffer);

        std::string finalMessage = std::format("[{}] [{}] {}",
                                               timestamp, levelStr, messageBuffer);

        Console::Get().AddLog(finalMessage.c_str());
    }
#endif

}
