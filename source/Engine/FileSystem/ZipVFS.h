#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <cstdint>
#include <memory>
#include <optional>

// Forward-declare zip struct (kuba--/zip)
struct zip_t;

class ZipVFS {
public:
    struct SourceEntry {
        std::string zipPath;   // full path to the zip file (used to resolve priority)
        int entryIndex;        // index inside the zip (zip_entry_openbyindex)
        uint64_t uncompressedSize;
        std::time_t mtime;     // fallback to zip file mtime if entry has no mtime
    };

    // Singleton-style usage
    static ZipVFS& Instance();

    // Initialize: scan 'rootPath' (recursively) for *.zip and *.pak and build index.
    // Returns true on success; false on failure (per your requirement, fail init on corrupt zip).
    bool Init(const std::string& rootPath);

    // Read text or binary file. If not found returns empty optional.
    // If file exists both on disk and in zip, disk has priority (caller should check disk first).
    std::optional<std::string> ReadFileAsText(const std::string& virtualPath);
    std::optional<std::vector<uint8_t>> ReadFileAsBinary(const std::string& virtualPath);

    // Returns all files (only immediate entries) in a directory path (non-recursive).
    // Example: virtualDir = "GameData/maps" or "maps" depending on your FS usage.
    std::vector<std::string> GetFilesInPath(const std::string& virtualDir);

    // Returns modification time for a file either from disk (if found) or zip entry's stored mtime.
    // Returns empty optional when not found.
    uint32_t GetFileModificationTime(const std::string& virtualPath);

    // Clear index (for shutdown or reindex). Not exposed publicly in initial design but useful in tests.
    void Shutdown();

private:
    ZipVFS() = default;
    ~ZipVFS() = default;

    // Non-copyable
    ZipVFS(const ZipVFS&) = delete;
    ZipVFS& operator=(const ZipVFS&) = delete;

    // internal helpers
    bool scanForZips(const std::string& rootPath, std::vector<std::string>& outZipPaths);
    bool indexSingleZip(const std::string& rootPath,
        const std::string& zipPath);


    // read raw bytes from a specific zip/entry source
    std::optional<std::vector<uint8_t>> readFromZip(const SourceEntry& src);

    // map: virtual path (like "GameData/maps/test.bsp" or maybe "maps/test.bsp") -> vector of sources
    // vector is ordered by zipPath alphabetical (lowest index = highest priority among zips).
    std::unordered_map<std::string, std::vector<SourceEntry>> m_index;

    // list of zip files (sorted alphabetically by full path)
    std::vector<std::string> m_zipFiles;

    // mutex for thread-safety for index and reading
    std::mutex m_mutex;
};
