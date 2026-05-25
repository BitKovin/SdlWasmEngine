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

        std::string finalMessage = std::format("[{}] [{}] {}",
            timestamp, levelStr, messageBuffer);

#ifdef __EMSCRIPTEN__
        // Bypasses the expensive std::cout stream buffer operations in WebAssembly
        printf("%s\n", finalMessage.c_str());
#else
        // Retain standard C++ logging for native desktop builds
        std::cout << finalMessage << "\n";
#endif

        Console::Get().AddLog(finalMessage.c_str());
    }
#endif
}