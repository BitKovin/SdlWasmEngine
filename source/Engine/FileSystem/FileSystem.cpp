#include "FileSystem.h"
#include "NativeFileSystem.h"
#include "ZipVFS.h"
#include "../Logger.hpp"

#include <mutex>
#include <algorithm>
#include <stdexcept>

namespace FileSystemEngine
{
    static std::vector<std::shared_ptr<IFileSystem>> g_fileSystems;
    static std::mutex g_fsMutex;

    void AddFileSystem(std::shared_ptr<IFileSystem> fs) {
        std::lock_guard<std::mutex> lock(g_fsMutex);
        g_fileSystems.push_back(fs);
    }

    void Init() {
        auto nativeFS = std::make_shared<NativeFileSystem>();
        if (!nativeFS->Init()) {
            Logger::Log("NativeFileSystem initialization failed");
            throw std::runtime_error("NativeFileSystem initialization failed");
        }
        AddFileSystem(nativeFS);

        auto zipFS = std::make_shared<ZipVFS>(nativeFS.get(), "GameData/");
        if (!zipFS->Init()) {
            Logger::Log("ZipVFS initialization failed");
            throw std::runtime_error("ZipVFS initialization failed");
        }
        AddFileSystem(zipFS);
    }

    void Shutdown() {
        std::lock_guard<std::mutex> lock(g_fsMutex);
        for (auto& fs : g_fileSystems) {
            fs->Shutdown();
        }
        g_fileSystems.clear();
    }

    std::string ReadFile(const std::string& path) {
        std::lock_guard<std::mutex> lock(g_fsMutex);
        if (g_fileSystems.empty()) return {};

        // Explicit route for SaveData -> Target Highest Priority FS implementation (Index 0)
        if (path.find("SaveData/") == 0) {
            return g_fileSystems.front()->ReadSaveFile(path).value_or(std::string{});
        }

        // Standard VFS cascade lookup
        for (auto& fs : g_fileSystems) {
            auto res = fs->ReadFile(path);
            if (res) return *res;
        }
        return {};
    }

    std::vector<uint8_t> ReadFileBinary(const std::string& path) {
        std::lock_guard<std::mutex> lock(g_fsMutex);
        if (g_fileSystems.empty()) return {};

        if (path.find("SaveData/") == 0) {
            return g_fileSystems.front()->ReadSaveFileBinary(path).value_or(std::vector<uint8_t>{});
        }

        for (auto& fs : g_fileSystems) {
            auto res = fs->ReadFileBinary(path);
            if (res) return *res;
        }
        return {};
    }

    bool WriteFile(const std::string& path, const std::string& content) {
        std::lock_guard<std::mutex> lock(g_fsMutex);
        if (g_fileSystems.empty()) return false;

        // Writes strictly route to the overriding root file system
        return g_fileSystems.front()->WriteSaveFile(path, content);
    }

    bool WriteFileBinary(const std::string& path, const std::vector<uint8_t>& data) {
        std::lock_guard<std::mutex> lock(g_fsMutex);
        if (g_fileSystems.empty()) return false;

        return g_fileSystems.front()->WriteSaveFileBinary(path, data);
    }

    std::vector<std::string> GetFilesInPath(const std::string& path) {
        std::lock_guard<std::mutex> lock(g_fsMutex);
        std::vector<std::string> combined;

        for (auto& fs : g_fileSystems) {
            auto files = fs->GetFilesInPath(path);
            for (const auto& f : files) {
                if (std::find(combined.begin(), combined.end(), f) == combined.end()) {
                    combined.push_back(f);
                }
            }
        }
        return combined;
    }

    uint32_t GetFileModificationTime(const std::string& path) {
        std::lock_guard<std::mutex> lock(g_fsMutex);
        for (auto& fs : g_fileSystems) {
            uint32_t t = fs->GetFileModificationTime(path);
            if (t != 0) return t;
        }
        return 0;
    }

} // namespace FileSystemEngine