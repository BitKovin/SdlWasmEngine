#include "ZipVFS.h"
#include <SDL2/SDL.h>
#include <algorithm>
#include <iostream>
#include "../Compression/zip/zip.h"
#include <Compression/miniz.h>

ZipArchiveHandle::~ZipArchiveHandle() {
    if (archive) zip_close(archive);
}

ZipArchiveHandle::ZipArchiveHandle(ZipArchiveHandle&& other) noexcept {
    archive = other.archive;
    memoryBuffer = std::move(other.memoryBuffer);
    other.archive = nullptr;
}

ZipArchiveHandle& ZipArchiveHandle::operator=(ZipArchiveHandle&& other) noexcept {
    if (this != &other) {
        if (archive) zip_close(archive);
        archive = other.archive;
        memoryBuffer = std::move(other.memoryBuffer);
        other.archive = nullptr;
    }
    return *this;
}

ZipVFS::ZipVFS(IFileSystem* backend, const std::string& rootPath)
    : m_backend(backend), m_rootPath(rootPath) {}

ZipVFS::~ZipVFS() { Shutdown(); }

bool ZipVFS::Init() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_index.clear();
    m_zipFiles.clear();

    if (!m_backend) {
        SDL_Log("ZipVFS::Init: no backend filesystem set");
        return false;
    }

    std::vector<std::string> foundZips;
    if (!scanForZips(foundZips)) {
        SDL_Log("ZipVFS::Init: scanForZips threw while walking '%s'", m_rootPath.c_str());
        return false;
    }

    SDL_Log("ZipVFS::Init: found %zu archive(s) under '%s'", foundZips.size(), m_rootPath.c_str());
    std::sort(foundZips.begin(), foundZips.end());

    // Index each archive independently -- one corrupt or unreadable archive
    // (or a single bad entry within it, see indexSingleZip) should not
    // discard everything successfully indexed from the others.
    for (const auto& zp : foundZips) {
        if (indexSingleZip(zp, "", 0)) {
            m_zipFiles.push_back(zp);
        } else {
            SDL_Log("ZipVFS::Init: failed to index '%s' -- skipping it, other archives unaffected", zp.c_str());
        }
    }

    SDL_Log("ZipVFS::Init: indexed %zu archive(s), %zu virtual path(s) total", m_zipFiles.size(), m_index.size());
    return true;
}

void ZipVFS::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_index.clear();
    m_zipFiles.clear();
}

bool ZipVFS::scanForZips(std::vector<std::string>& outZipPaths) {
    try {
        std::vector<std::string> dirsToScan = { m_rootPath };
        while (!dirsToScan.empty()) {
            std::string currentDir = dirsToScan.back();
            dirsToScan.pop_back();

            auto entries = m_backend->GetFilesInPath(currentDir);
            for (const auto& entry : entries) {
                std::string fullPath = currentDir;
                if (!fullPath.empty() && fullPath.back() != '/') fullPath += '/';
                fullPath += entry;

                if (m_backend->IsDirectory(fullPath)) dirsToScan.push_back(fullPath);
                else if (isZipFile(entry)) outZipPaths.push_back(fullPath);
            }
        }
        return true;
    }
    catch (const std::exception& e) {
        SDL_Log("ZipVFS::scanForZips: exception while walking '%s': %s", m_rootPath.c_str(), e.what());
        return false;
    }
    catch (...) {
        SDL_Log("ZipVFS::scanForZips: unknown exception while walking '%s'", m_rootPath.c_str());
        return false;
    }
}

bool ZipVFS::isZipFile(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) return false;
    std::string ext = filename.substr(dotPos);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return (ext == ".zip" || ext == ".pak");
}

ZipArchiveHandle ZipVFS::openZip(const std::string& zipPath) {
    ZipArchiveHandle handle;
    std::string physical = m_backend->GetPhysicalPath(zipPath);

    if (!physical.empty()) {
        handle.archive = zip_open(physical.c_str(), 0, 'r');
        if (!handle.archive) {
            SDL_Log("ZipVFS::openZip: zip_open('%s') failed", physical.c_str());
        }
    }
    else {
        auto dataOpt = m_backend->ReadFileBinary(zipPath);
        if (!dataOpt) {
            SDL_Log("ZipVFS::openZip: backend->ReadFileBinary('%s') failed", zipPath.c_str());
        } else {
            handle.memoryBuffer = std::move(*dataOpt);
            handle.archive = zip_stream_open(reinterpret_cast<const char*>(handle.memoryBuffer.data()), handle.memoryBuffer.size(), 0, 'r');
            if (!handle.archive) {
                SDL_Log("ZipVFS::openZip: zip_stream_open('%s', %zu bytes) failed", zipPath.c_str(), handle.memoryBuffer.size());
            }
        }
    }
    return handle;
}

bool ZipVFS::indexSingleZip(const std::string& zipPath, const std::string& virtualPrefix, int nestingDepth) {
    if (nestingDepth > 10) {
        SDL_Log("ZipVFS::indexSingleZip: '%s' exceeds max nesting depth (10) -- skipping", zipPath.c_str());
        return false;
    }

    ZipArchiveHandle handle = openZip(zipPath);
    if (!handle.archive) {
        SDL_Log("ZipVFS::indexSingleZip: couldn't open '%s'", zipPath.c_str());
        return false;
    }

    std::string entryPrefix = virtualPrefix;
    if (nestingDepth == 0) {
        entryPrefix = m_rootPath;
        if (!entryPrefix.empty() && entryPrefix.back() != '/') entryPrefix += "/";
        std::string relDir = "";
        if (zipPath.rfind(entryPrefix, 0) == 0) {
            relDir = zipPath.substr(entryPrefix.size());
            size_t lastSlash = relDir.rfind('/');
            relDir = (lastSlash != std::string::npos) ? relDir.substr(0, lastSlash + 1) : "";
        }
        entryPrefix += relDir;
    }

    int total = zip_entries_total(handle.archive);
    if (total < 0) total = 0;
    SDL_Log("ZipVFS::indexSingleZip: '%s' opened, %d entr%s", zipPath.c_str(), total, total == 1 ? "y" : "ies");

    std::vector<NestedZipEntry> nestedZips;
    for (int i = 0; i < total; ++i) {
        if (zip_entry_openbyindex(handle.archive, i) < 0) {
            SDL_Log("ZipVFS::indexSingleZip: '%s' entry %d failed to open -- skipping entry", zipPath.c_str(), i);
            continue;
        }

        const char* name = zip_entry_name(handle.archive);
        if (!name) { zip_entry_close(handle.archive); continue; }

        std::string entryName(name);
        std::string vpath = entryPrefix + entryName;
        if (vpath.rfind("./", 0) == 0) vpath = vpath.substr(2);

        if (isZipFile(entryName)) {
            nestedZips.push_back({ vpath, zipPath, i, entryName });
        }

        SourceEntry src;
        src.zipPath = zipPath;
        src.entryIndex = i;
        src.uncompressedSize = static_cast<uint64_t>(zip_entry_size(handle.archive));

        mz_zip_archive* archive = reinterpret_cast<mz_zip_archive*>(handle.archive);
        mz_zip_archive_file_stat fileStat{};
        src.mtime = mz_zip_reader_file_stat(archive, static_cast<mz_uint>(i), &fileStat) ? static_cast<uint32_t>(fileStat.m_time) : 0;
        src.isNested = false;

        m_index[vpath].push_back(src);
        auto& vec = m_index[vpath];
        std::sort(vec.begin(), vec.end(), [](const SourceEntry& a, const SourceEntry& b) { return a.zipPath > b.zipPath; });

        zip_entry_close(handle.archive);
    }

    for (const auto& nested : nestedZips) {
        if (!extractAndIndexNestedZip(nested, nestingDepth + 1)) {
            SDL_Log("ZipVFS::indexSingleZip: failed to index nested archive '%s' inside '%s' -- skipping",
                    nested.virtualPath.c_str(), zipPath.c_str());
        }
    }
    return true;
}

bool ZipVFS::extractAndIndexNestedZip(const NestedZipEntry& nested, int currentDepth) {
    ZipArchiveHandle parentHandle = openZip(nested.parentZipPath);
    if (!parentHandle.archive) {
        SDL_Log("ZipVFS::extractAndIndexNestedZip: couldn't reopen parent '%s' for nested '%s'",
                nested.parentZipPath.c_str(), nested.virtualPath.c_str());
        return false;
    }

    if (zip_entry_openbyindex(parentHandle.archive, nested.entryIndex) < 0) {
        SDL_Log("ZipVFS::extractAndIndexNestedZip: couldn't reopen entry %d in '%s' for nested '%s'",
                nested.entryIndex, nested.parentZipPath.c_str(), nested.virtualPath.c_str());
        return false;
    }
    void* buf = nullptr; size_t bufsize = 0;
    int r = zip_entry_read(parentHandle.archive, &buf, &bufsize);
    zip_entry_close(parentHandle.archive);

    if (r < 0 || !buf) {
        SDL_Log("ZipVFS::extractAndIndexNestedZip: couldn't read nested archive bytes for '%s'", nested.virtualPath.c_str());
        return false;
    }

    ZipArchiveHandle nestedHandle;
    nestedHandle.memoryBuffer.assign(static_cast<uint8_t*>(buf), static_cast<uint8_t*>(buf) + bufsize);
    free(buf);

    nestedHandle.archive = zip_stream_open(reinterpret_cast<const char*>(nestedHandle.memoryBuffer.data()), nestedHandle.memoryBuffer.size(), 0, 'r');
    if (!nestedHandle.archive) {
        SDL_Log("ZipVFS::extractAndIndexNestedZip: zip_stream_open failed for nested '%s' (%zu bytes)",
                nested.virtualPath.c_str(), nestedHandle.memoryBuffer.size());
        return false;
    }

    std::string nestedPrefix = nested.virtualPath;
    size_t lastSlash = nestedPrefix.rfind('/');
    nestedPrefix = (lastSlash != std::string::npos) ? nestedPrefix.substr(0, lastSlash + 1) : "";

    int total = zip_entries_total(nestedHandle.archive);
    if (total < 0) total = 0;

    for (int i = 0; i < total; ++i) {
        if (zip_entry_openbyindex(nestedHandle.archive, i) < 0) {
            SDL_Log("ZipVFS::extractAndIndexNestedZip: '%s' entry %d failed to open -- skipping entry",
                    nested.virtualPath.c_str(), i);
            continue;
        }

        const char* name = zip_entry_name(nestedHandle.archive);
        if (!name) { zip_entry_close(nestedHandle.archive); continue; }

        std::string entryName(name);
        std::string vpath = nestedPrefix + entryName;
        if (vpath.rfind("./", 0) == 0) vpath = vpath.substr(2);

        SourceEntry src;
        src.zipPath = nested.parentZipPath;
        src.entryIndex = i;
        src.parentZipPath = nested.parentZipPath;
        src.parentEntryIndex = nested.entryIndex;
        src.isNested = true;
        src.uncompressedSize = static_cast<uint64_t>(zip_entry_size(nestedHandle.archive));

        mz_zip_archive* archive = reinterpret_cast<mz_zip_archive*>(nestedHandle.archive);
        mz_zip_archive_file_stat fileStat{};
        src.mtime = mz_zip_reader_file_stat(archive, static_cast<mz_uint>(i), &fileStat) ? static_cast<uint32_t>(fileStat.m_time) : 0;

        m_index[vpath].push_back(src);
        auto& vec = m_index[vpath];
        std::sort(vec.begin(), vec.end(), [](const SourceEntry& a, const SourceEntry& b) { return a.zipPath > b.zipPath; });

        zip_entry_close(nestedHandle.archive);
    }
    return true;
}

std::optional<std::vector<uint8_t>> ZipVFS::readFromZip(const SourceEntry& src) {
    // Open parent zip. (If it's nested, open the parent zip. If not, open the direct zip).
    ZipArchiveHandle parentHandle = openZip(src.isNested ? src.parentZipPath : src.zipPath);
    if (!parentHandle.archive) return std::nullopt;

    zip_t* targetArchive = parentHandle.archive;
    ZipArchiveHandle nestedHandle;

    if (src.isNested) {
        if (zip_entry_openbyindex(parentHandle.archive, src.parentEntryIndex) < 0) return std::nullopt;
        void* buf = nullptr; size_t bufsize = 0;
        int r = zip_entry_read(parentHandle.archive, &buf, &bufsize);
        zip_entry_close(parentHandle.archive);

        if (r < 0 || !buf) return std::nullopt;

        nestedHandle.memoryBuffer.assign(static_cast<uint8_t*>(buf), static_cast<uint8_t*>(buf) + bufsize);
        free(buf);

        nestedHandle.archive = zip_stream_open(reinterpret_cast<const char*>(nestedHandle.memoryBuffer.data()), nestedHandle.memoryBuffer.size(), 0, 'r');
        if (!nestedHandle.archive) return std::nullopt;
        targetArchive = nestedHandle.archive;
    }

    if (zip_entry_openbyindex(targetArchive, src.entryIndex) < 0) return std::nullopt;
    void* fileBuf = nullptr; size_t fileBufSize = 0;
    int r = zip_entry_read(targetArchive, &fileBuf, &fileBufSize);
    zip_entry_close(targetArchive);

    if (r < 0 || !fileBuf) return std::nullopt;

    std::vector<uint8_t> out(fileBufSize);
    memcpy(out.data(), fileBuf, fileBufSize);
    free(fileBuf);

    return out;
}

std::optional<std::string> ZipVFS::ReadFile(const std::string& virtualPath) {
    auto binOpt = ReadFileBinary(virtualPath);
    if (!binOpt) return std::nullopt;
    return std::string(binOpt->begin(), binOpt->end());
}

std::optional<std::vector<uint8_t>> ZipVFS::ReadFileBinary(const std::string& virtualPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_index.find(virtualPath);
    if (it == m_index.end() || it->second.empty()) return std::nullopt;

    for (const auto& src : it->second) {
        auto readRes = readFromZip(src);
        if (readRes) return readRes;
    }
    return std::nullopt;
}

std::vector<std::string> ZipVFS::GetFilesInPath(const std::string& virtualDir) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> out;
    std::string prefix = virtualDir;
    if (!prefix.empty() && prefix.back() != '/') prefix += '/';

    for (const auto& kv : m_index) {
        if (kv.first.rfind(prefix, 0) != 0) continue;

        std::string rest = kv.first.substr(prefix.size());
        if (rest.empty()) continue;

        auto pos = rest.find('/');
        std::string entryName = (pos == std::string::npos) ? rest : rest.substr(0, pos);

        if (std::find(out.begin(), out.end(), entryName) == out.end()) {
            out.push_back(entryName);
        }
    }
    return out;
}

bool ZipVFS::IsDirectory(const std::string& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::string prefix = path;
    if (!prefix.empty() && prefix.back() != '/') prefix += '/';

    for (const auto& kv : m_index) {
        if (kv.first.rfind(prefix, 0) == 0) return true;
    }
    return false;
}

uint32_t ZipVFS::GetFileModificationTime(const std::string& virtualPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_index.find(virtualPath);
    if (it == m_index.end() || it->second.empty()) return 0;
    return it->second.front().mtime;
}