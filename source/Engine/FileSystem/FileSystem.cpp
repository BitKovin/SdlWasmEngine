#include "FileSystem.h"
#include "ZipVFS.h"

#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <optional>
#include <chrono>
#include <mutex>

#include "../Logger.hpp"

namespace fs = std::filesystem;

static std::mutex g_fsMutex;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/emscripten.h>

/*
    Emscripten-backed write helpers.
    ZIP IS NOT SUPPORTED FOR WRITING.
*/

EM_JS(void, WriteFileJS, (const char* relPath, const char* content), {
    const path = UTF8ToString(relPath);
    const data = UTF8ToString(content);

    try {
        const parts = path.split('/');
        let current = '';
        for (let i = 0; i < parts.length - 1; i++) {
            current += (i ? '/' : '') + parts[i];
            if (!FS.analyzePath(current).exists) {
                FS.mkdir(current);
            }
        }
        FS.writeFile(path, data);
    }
 catch (e) {
  console.error("WriteFileJS failed:", e);
}
    });

EM_JS(void, WriteFileBinaryJS, (const char* relPath, const uint8_t* data, int size), {
    const path = UTF8ToString(relPath);
    const buffer = new Uint8Array(Module.HEAPU8.buffer, data, size);

    try {
        const parts = path.split('/');
        let current = '';
        for (let i = 0; i < parts.length - 1; i++) {
            current += (i ? '/' : '') + parts[i];
            if (!FS.analyzePath(current).exists) {
                FS.mkdir(current);
            }
        }
        FS.writeFile(path, buffer);
    }
 catch (e) {
  console.error("WriteFileBinaryJS failed:", e);
}
    });
#endif

/* ============================
   INITIALIZATION
   ============================ */

void FileSystemEngine::Init()
{
    // ZIP VFS is read-only and indexed once
    if (!ZipVFS::Instance().Init("GameData/"))
    {
        Logger::Log("ZipVFS initialization failed");
        throw std::runtime_error("ZipVFS initialization failed");
    }
}

void FileSystemEngine::Shutdown()
{
	ZipVFS::Instance().Shutdown();
}

/* ============================
   READ FILE (TEXT)
   ============================ */

std::string FileSystemEngine::ReadFile(const std::string& path)
{
    std::lock_guard<std::mutex> lock(g_fsMutex);

    // 1. Disk always has priority
    try
    {
        if (fs::exists(path) && fs::is_regular_file(path))
        {
            std::ifstream file(path, std::ios::binary);
            if (!file)
                return {};

            file.seekg(0, std::ios::end);
            size_t size = static_cast<size_t>(file.tellg());
            file.seekg(0);

            std::string result(size, '\0');
            file.read(result.data(), size);
            return result;
        }
    }
    catch (...) {}

    // 2. ZIP fallback
    auto zipResult = ZipVFS::Instance().ReadFileAsText(path);
    if (zipResult)
        return *zipResult;

    return {};
}

/* ============================
   READ FILE (BINARY)
   ============================ */

std::vector<uint8_t> FileSystemEngine::ReadFileBinary(const std::string& path)
{
    std::lock_guard<std::mutex> lock(g_fsMutex);

    // Disk first
    try
    {
        if (fs::exists(path) && fs::is_regular_file(path))
        {
            std::ifstream file(path, std::ios::binary);
            if (!file)
                return {};

            file.seekg(0, std::ios::end);
            size_t size = static_cast<size_t>(file.tellg());
            file.seekg(0);

            std::vector<uint8_t> data(size);
            file.read(reinterpret_cast<char*>(data.data()), size);
            return data;
        }
    }
    catch (...) {}

    // ZIP fallback
    auto zipResult = ZipVFS::Instance().ReadFileAsBinary(path);
    if (zipResult)
        return *zipResult;

    return {};
}

/* ============================
   WRITE FILE (TEXT)
   ZIP IS NOT SUPPORTED
   ============================ */

bool FileSystemEngine::WriteFile(const std::string& path, const std::string& content)
{
    std::lock_guard<std::mutex> lock(g_fsMutex);

#ifdef __EMSCRIPTEN__
    WriteFileJS(path.c_str(), content.c_str());
    return true;
#else
    try
    {
        fs::create_directories(fs::path(path).parent_path());
        std::ofstream file(path, std::ios::binary);
        if (!file)
            return false;

        file.write(content.data(), content.size());
        return true;
    }
    catch (...)
    {
        return false;
    }
#endif
}

/* ============================
   WRITE FILE (BINARY)
   ZIP IS NOT SUPPORTED
   ============================ */

bool FileSystemEngine::WriteFileBinary(const std::string& path, const std::vector<uint8_t>& data)
{
    std::lock_guard<std::mutex> lock(g_fsMutex);

#ifdef __EMSCRIPTEN__
    WriteFileBinaryJS(path.c_str(), data.data(), static_cast<int>(data.size()));
    return true;
#else
    try
    {
        fs::create_directories(fs::path(path).parent_path());
        std::ofstream file(path, std::ios::binary);
        if (!file)
            return false;

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return true;
    }
    catch (...)
    {
        return false;
    }
#endif
}

/* ============================
   DIRECTORY LISTING
   ============================ */

std::vector<std::string> FileSystemEngine::GetFilesInPath(const std::string& path)
{
    std::lock_guard<std::mutex> lock(g_fsMutex);
    std::vector<std::string> result;

    // Disk
    try
    {
        if (fs::exists(path) && fs::is_directory(path))
        {
            for (const auto& entry : fs::directory_iterator(path))
            {
                result.push_back(entry.path().filename().string());
            }
        }
    }
    catch (...) {}

    // ZIP
    auto zipEntries = ZipVFS::Instance().GetFilesInPath(path);
    for (const auto& name : zipEntries)
    {
        if (std::find(result.begin(), result.end(), name) == result.end())
        {
            result.push_back(name);
        }
    }

    return result;
}

/* ============================
   FILE MODIFICATION TIME
   ============================ */

std::uint32_t FileSystemEngine::GetFileModificationTime(const std::string& path)
{
    std::lock_guard<std::mutex> lock(g_fsMutex);

    // Disk priority
    try
    {
        if (fs::exists(path) && fs::is_regular_file(path))
        {
            auto ftime = fs::last_write_time(path);

            // C++17-safe conversion
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now()
                + std::chrono::system_clock::now()
            );

            return std::chrono::system_clock::to_time_t(sctp);
        }
    }
    catch (...) {}

    // ZIP fallback
    return ZipVFS::Instance().GetFileModificationTime(path);
}
