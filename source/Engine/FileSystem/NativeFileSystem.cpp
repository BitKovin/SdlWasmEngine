#include "NativeFileSystem.h"
#include <filesystem>
#include <fstream>
#include <chrono>

namespace fs = std::filesystem;

bool NativeFileSystem::Init() { return true; }
void NativeFileSystem::Shutdown() {}

std::optional<std::string> NativeFileSystem::ReadFile(const std::string& path) {
    try {
        if (!fs::exists(path) || !fs::is_regular_file(path)) return std::nullopt;
        std::ifstream file(path, std::ios::binary);
        if (!file) return std::nullopt;

        file.seekg(0, std::ios::end);
        size_t size = static_cast<size_t>(file.tellg());
        file.seekg(0);

        std::string result(size, '\0');
        file.read(result.data(), size);
        return result;
    }
    catch (...) { return std::nullopt; }
}

std::optional<std::vector<uint8_t>> NativeFileSystem::ReadFileBinary(const std::string& path)
{
    try
    {
        fs::path fullPath = fs::absolute(path);

        if (!fs::exists(fullPath))
        {
            printf("[FileSystem] FILE NOT FOUND: %s\n", fullPath.string().c_str());
            return std::nullopt;
        }

        if (!fs::is_regular_file(fullPath))
        {
            printf("[FileSystem] NOT A REGULAR FILE: %s\n", fullPath.string().c_str());
            return std::nullopt;
        }

        std::ifstream file(fullPath, std::ios::binary);
        if (!file.is_open())
        {
            printf("[FileSystem] FAILED TO OPEN FILE: %s\n", fullPath.string().c_str());
            return std::nullopt;
        }

        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();

        if (size <= 0)
        {
            printf("[FileSystem] INVALID FILE SIZE (%lld): %s\n",
                (long long)size,
                fullPath.string().c_str());
            return std::nullopt;
        }

        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> data(static_cast<size_t>(size));

        if (!file.read(reinterpret_cast<char*>(data.data()), size))
        {
            printf("[FileSystem] FAILED TO READ FILE: %s\n", fullPath.string().c_str());
            return std::nullopt;
        }

        return data;
    }
    catch (const std::exception& e)
    {
        printf("[FileSystem] EXCEPTION reading %s: %s\n",
            fs::absolute(path).string().c_str(),
            e.what());
        return std::nullopt;
    }
    catch (...)
    {
        printf("[FileSystem] UNKNOWN EXCEPTION reading: %s\n",
            fs::absolute(path).string().c_str());
        return std::nullopt;
    }
}
std::vector<std::string> NativeFileSystem::GetFilesInPath(const std::string& path) {
    std::vector<std::string> result;
    try {
        if (fs::exists(path) && fs::is_directory(path)) {
            for (const auto& entry : fs::directory_iterator(path)) {
                result.push_back(entry.path().filename().string());
            }
        }
    }
    catch (...) {}
    return result;
}

bool NativeFileSystem::IsDirectory(const std::string& path) {
    try { return fs::exists(path) && fs::is_directory(path); }
    catch (...) { return false; }
}

uint32_t NativeFileSystem::GetFileModificationTime(const std::string& path) {
    try {
        if (fs::exists(path) && fs::is_regular_file(path)) {
            auto ftime = fs::last_write_time(path);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
            );
            return static_cast<uint32_t>(std::chrono::system_clock::to_time_t(sctp));
        }
    }
    catch (...) {}
    return 0;
}

bool NativeFileSystem::WriteSaveFile(const std::string& path, const std::string& content) {
    try {
        fs::create_directories(fs::path(path).parent_path());
        std::ofstream file(path, std::ios::binary);
        if (!file) return false;
        file.write(content.data(), content.size());
        return true;
    }
    catch (...) { return false; }
}

bool NativeFileSystem::WriteSaveFileBinary(const std::string& path, const std::vector<uint8_t>& data) {
    try {
        fs::create_directories(fs::path(path).parent_path());
        std::ofstream file(path, std::ios::binary);
        if (!file) return false;
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return true;
    }
    catch (...) { return false; }
}

std::optional<std::string> NativeFileSystem::ReadSaveFile(const std::string& path) { return ReadFile(path); }
std::optional<std::vector<uint8_t>> NativeFileSystem::ReadSaveFileBinary(const std::string& path) { return ReadFileBinary(path); }
std::string NativeFileSystem::GetPhysicalPath(const std::string& path) { return path; }