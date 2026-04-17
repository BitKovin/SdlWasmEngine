#pragma once
#include "IFileSystem.h"

class NativeFileSystem : public IFileSystem {
public:
    NativeFileSystem() = default;
    ~NativeFileSystem() override = default;

    bool Init() override;
    void Shutdown() override;

    std::optional<std::string> ReadFile(const std::string& path) override;
    std::optional<std::vector<uint8_t>> ReadFileBinary(const std::string& path) override;
    std::vector<std::string> GetFilesInPath(const std::string& path) override;
    bool IsDirectory(const std::string& path) override;
    uint32_t GetFileModificationTime(const std::string& path) override;

    bool WriteSaveFile(const std::string& path, const std::string& content) override;
    bool WriteSaveFileBinary(const std::string& path, const std::vector<uint8_t>& data) override;
    std::optional<std::string> ReadSaveFile(const std::string& path) override;
    std::optional<std::vector<uint8_t>> ReadSaveFileBinary(const std::string& path) override;

    std::string GetPhysicalPath(const std::string& path) override;
};