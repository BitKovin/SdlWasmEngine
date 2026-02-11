#pragma once

#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
#include <unordered_map>

// Forward declaration
struct zip_t;

struct SourceEntry {
    std::string zipPath;  // Can be file path or "memory://..." identifier
    int entryIndex;
    uint64_t uncompressedSize;
    uint32_t mtime;
    bool isNested = false;
};

struct NestedZipEntry {
    std::string virtualPath;
    std::string parentZipPath;  // Can be file path or "memory://..." identifier
    int entryIndex;
    std::string entryName;
};

class ZipVFS {
public:
    static ZipVFS& Instance();

    bool Init(const std::string& rootPath);
    void Shutdown();

    std::optional<std::string> ReadFileAsText(const std::string& virtualPath);
    std::optional<std::vector<uint8_t>> ReadFileAsBinary(const std::string& virtualPath);
    std::vector<std::string> GetFilesInPath(const std::string& virtualDir);
    uint32_t GetFileModificationTime(const std::string& virtualPath);

private:
    ZipVFS() = default;
    ~ZipVFS() { Shutdown(); }
    ZipVFS(const ZipVFS&) = delete;
    ZipVFS& operator=(const ZipVFS&) = delete;

    bool scanForZips(const std::string& rootPath, std::vector<std::string>& outZipPaths);
    bool indexSingleZip(const std::string& rootPath, const std::string& zipPath,
        const std::string& virtualPrefix, int nestingDepth);
    bool extractAndIndexNestedZip(const std::string& rootPath,
        const NestedZipEntry& nested,
        int currentDepth);
    bool isZipFile(const std::string& filename);
    zip_t* openZipFromPathOrMemory(const std::string& zipPath);

    std::optional<std::vector<uint8_t>> readFromZip(const SourceEntry& src);

    std::mutex m_mutex;
    std::unordered_map<std::string, std::vector<SourceEntry>> m_index;
    std::vector<std::string> m_zipFiles;
    std::unordered_map<std::string, std::vector<uint8_t>> m_nestedZipData;  // memory://<id> -> zip data
};