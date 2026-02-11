#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace FileSystemEngine
{

    void Init();

	void Shutdown();

    // Read binary file (GameData or SaveData)
    // on failure: throws std::runtime_error
    std::vector<uint8_t> ReadFileBinary(const std::string& path);

    // Read text file (GameData or SaveData)
    // on failure: throws std::runtime_error
    std::string ReadFile(const std::string& path);

    std::vector<std::string> GetFilesInPath(const std::string& path);

    // Returns last modification time as Unix timestamp (seconds since epoch)
    // Returns 0 on failure or if file doesn't exist
    uint32_t GetFileModificationTime(const std::string& path);

    // Write text file (GameData is read‑only! only SaveData makes sense here)
    // returns true on success, false on failure
    bool WriteFile(const std::string& path, const std::string& content);

    bool WriteFileBinary(const std::string& path, const std::vector<uint8_t>& data);

} // namespace FileSystem
