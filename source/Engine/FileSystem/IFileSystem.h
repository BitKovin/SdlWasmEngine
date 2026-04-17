#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

class IFileSystem {
public:
    virtual ~IFileSystem() = default;

    virtual bool Init() = 0;
    virtual void Shutdown() = 0;

    // Standard VFS
    virtual std::optional<std::string> ReadFile(const std::string& path) = 0;
    virtual std::optional<std::vector<uint8_t>> ReadFileBinary(const std::string& path) = 0;
    virtual std::vector<std::string> GetFilesInPath(const std::string& path) = 0;
    virtual bool IsDirectory(const std::string& path) = 0;
    virtual uint32_t GetFileModificationTime(const std::string& path) = 0;

    // Save Data Handlers (overridden by platform-specific subclasses like Emscripten)
    virtual bool WriteSaveFile(const std::string& path, const std::string& content) = 0;
    virtual bool WriteSaveFileBinary(const std::string& path, const std::vector<uint8_t>& data) = 0;
    virtual std::optional<std::string> ReadSaveFile(const std::string& path) = 0;
    virtual std::optional<std::vector<uint8_t>> ReadSaveFileBinary(const std::string& path) = 0;

    // Optimization hint: returns actual disk path if physical, or empty string if virtual
    virtual std::string GetPhysicalPath(const std::string& path) { return ""; }
};