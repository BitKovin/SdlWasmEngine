#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <memory>
#include "IFileSystem.h"

namespace FileSystemEngine
{
    void Init();
    void Shutdown();

    // Dynamically push backend layers (first added handles SaveData routing overrides)
    void AddFileSystem(std::shared_ptr<IFileSystem> fs);

    std::vector<uint8_t> ReadFileBinary(const std::string& path);
    std::string ReadFile(const std::string& path);
    std::vector<std::string> GetFilesInPath(const std::string& path);
    uint32_t GetFileModificationTime(const std::string& path);

    bool WriteFile(const std::string& path, const std::string& content);
    bool WriteFileBinary(const std::string& path, const std::vector<uint8_t>& data);

} // namespace FileSystemEngine