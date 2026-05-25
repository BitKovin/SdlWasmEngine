#pragma once

#include <chrono>
#include <format>
#include <string>
#include <cstdio>
#include <cstdarg>
#include <mutex>
#include <type_traits>
#include <utility>

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

    template <typename... Args>
    static void Log(const char* format, Args&&... args)
    {
        LogVariadic(Level::Info, format, UnwrapArg(std::forward<Args>(args))...);
    }

    template <typename... Args>
    static void Info(const char* format, Args&&... args)
    {
        LogVariadic(Level::Info, format, UnwrapArg(std::forward<Args>(args))...);
    }

    template <typename... Args>
    static void Warning(const char* format, Args&&... args)
    {
        LogVariadic(Level::Warning, format, UnwrapArg(std::forward<Args>(args))...);
    }

    template <typename... Args>
    static void Error(const char* format, Args&&... args)
    {
        LogVariadic(Level::Error, format, UnwrapArg(std::forward<Args>(args))...);
    }

    template <typename... Args>
    static void Fatal(const char* format, Args&&... args)
    {
        LogVariadic(Level::Fatal, format, UnwrapArg(std::forward<Args>(args))...);
    }

private:
    // Helper to safely unpack std::string to .c_str() automatically
    template <typename T>
    static decltype(auto) UnwrapArg(T&& arg)
    {
        if constexpr (std::is_same_v<std::decay_t<T>, std::string>)
        {
            return arg.c_str();
        }
        else
        {
            return std::forward<T>(arg);
        }
    }

    // Bridge from templates back to C-style va_list
    static void LogVariadic(Level level, const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        LogInternal(level, format, args);
        va_end(args);
    }

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

    static inline std::mutex g_LogMutex;

    static void LogInternal(Level level, const char* format, va_list args);
};