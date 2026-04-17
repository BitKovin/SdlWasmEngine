#pragma once

#include "IFileSystem.h"
#include <mutex>
#include <unordered_map>
#include <memory>

struct zip_t;

struct SourceEntry {
    std::string zipPath;
    int entryIndex;

    // Support for nesting without persistent memory footprint
    bool isNested = false;
    std::string parentZipPath;
    int parentEntryIndex = -1;

    uint64_t uncompressedSize;
    uint32_t mtime;
};

struct NestedZipEntry {
    std::string virtualPath;
    std::string parentZipPath;
    int entryIndex;
    std::string entryName;
};

// RAII Wrapper to hold temporary in-memory zip files and their backing buffers
struct ZipArchiveHandle {
    zip_t* archive = nullptr;
    std::vector<uint8_t> memoryBuffer;

    ZipArchiveHandle() = default;
    ~ZipArchiveHandle();
    ZipArchiveHandle(const ZipArchiveHandle&) = delete;
    ZipArchiveHandle& operator=(const ZipArchiveHandle&) = delete;
    ZipArchiveHandle(ZipArchiveHandle&& other) noexcept;
    ZipArchiveHandle& operator=(ZipArchiveHandle&& other) noexcept;
};

class ZipVFS : public IFileSystem {
public:
    ZipVFS(IFileSystem* backend, const std::string& rootPath = "GameData/");
    ~ZipVFS() override;

    bool Init() override;
    void Shutdown() override;

    std::optional<std::string> ReadFile(const std::string& path) override;
    std::optional<std::vector<uint8_t>> ReadFileBinary(const std::string& path) override;
    std::vector<std::string> GetFilesInPath(const std::string& path) override;
    bool IsDirectory(const std::string& path) override;
    uint32_t GetFileModificationTime(const std::string& path) override;

    bool WriteSaveFile(const std::string& path, const std::string& content) override { return false; }
    bool WriteSaveFileBinary(const std::string& path, const std::vector<uint8_t>& data) override { return false; }
    std::optional<std::string> ReadSaveFile(const std::string& path) override { return std::nullopt; }
    std::optional<std::vector<uint8_t>> ReadSaveFileBinary(const std::string& path) override { return std::nullopt; }

private:
    bool scanForZips(std::vector<std::string>& outZipPaths);
    bool indexSingleZip(const std::string& zipPath, const std::string& virtualPrefix, int nestingDepth);
    bool extractAndIndexNestedZip(const NestedZipEntry& nested, int currentDepth);
    bool isZipFile(const std::string& filename);

    ZipArchiveHandle openZip(const std::string& zipPath);
    std::optional<std::vector<uint8_t>> readFromZip(const SourceEntry& src);

    IFileSystem* m_backend;
    std::string m_rootPath;

    std::mutex m_mutex;
    std::unordered_map<std::string, std::vector<SourceEntry>> m_index;
    std::vector<std::string> m_zipFiles;
};