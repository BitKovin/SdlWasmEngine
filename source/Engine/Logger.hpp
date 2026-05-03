#pragma once

#include <chrono>
#include <format>
#include <string>
#include <cstdio>
#include <cstdarg>

class Logger
{
public:

    enum class Level
    {
        Info,
        Warning,
        Error,
        Fatal
    };

    static void Log(const std::string& message)
    {
        Info("%s", message.c_str());
    }

    static void Log(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        LogInternal(Level::Info, format, args);
        va_end(args);
    }

    static void Info(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        LogInternal(Level::Info, format, args);
        va_end(args);
    }

    static void Warning(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        LogInternal(Level::Warning, format, args);
        va_end(args);
    }

    static void Error(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        LogInternal(Level::Error, format, args);
        va_end(args);
    }

    static void Fatal(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        LogInternal(Level::Fatal, format, args);
        va_end(args);
    }

private:

    static std::string GetTimestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto seconds = std::chrono::floor<std::chrono::seconds>(now);
        return std::format("{:%H:%M:%S}", seconds);
    }

    static const char* GetLevelString(Level level)
    {
        switch (level)
        {
        case Level::Info:    return "INFO";
        case Level::Warning: return "WARN";
        case Level::Error:   return "ERROR";
        case Level::Fatal:   return "FATAL";
        default:             return "UNKNOWN";
        }
    }

    static void LogInternal(Level level, const char* format, va_list args)
    {
#ifndef DISTRIBUTION
        char messageBuffer[1024];
        vsnprintf(messageBuffer, sizeof(messageBuffer), format, args);

        printf("[%s] [%s] %s\n",
            GetTimestamp().c_str(),
            GetLevelString(level),
            messageBuffer);
#endif
    }
};