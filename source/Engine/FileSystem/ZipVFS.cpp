#include "ZipVFS.h"
#include <algorithm>
#include <iostream>
#include <mutex>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include "../Compression/zip/zip.h"

namespace fs = std::filesystem;

ZipVFS& ZipVFS::Instance() {
    static ZipVFS inst;
    return inst;
}

bool ZipVFS::Init(const std::string& rootPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_index.clear();
    m_zipFiles.clear();
    m_nestedZipData.clear();

    std::vector<std::string> foundZips;
    if (!scanForZips(rootPath, foundZips)) {
        std::cerr << "ZipVFS: failed scanning for zips under: " << rootPath << "\n";
        return false;
    }

    std::sort(foundZips.begin(), foundZips.end());
    m_zipFiles = foundZips;

    // index each zip with nesting support
    for (const auto& zp : m_zipFiles) {
        if (!indexSingleZip(rootPath, zp, "", 0)) {
            std::cerr << "ZipVFS: failed indexing zip: " << zp << "\n";
            m_index.clear();
            m_zipFiles.clear();
            m_nestedZipData.clear();
            return false;
        }
    }

    return true;
}

void ZipVFS::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_index.clear();
    m_zipFiles.clear();
    m_nestedZipData.clear();
}

bool ZipVFS::scanForZips(const std::string& rootPath, std::vector<std::string>& outZipPaths) {
    try {
        if (!fs::exists(rootPath)) return true;

        for (auto const& dirEntry : fs::recursive_directory_iterator(rootPath)) {
            if (!dirEntry.is_regular_file()) continue;

            auto ext = dirEntry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c) { return std::tolower(c); });

            if (ext == ".zip" || ext == ".pak") {
                outZipPaths.push_back(dirEntry.path().string());
            }
        }
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "ZipVFS: exception while scanning zips: " << e.what() << "\n";
        return false;
    }
}

bool ZipVFS::isZipFile(const std::string& filename) {
    auto ext = fs::path(filename).extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return (ext == ".zip" || ext == ".pak");
}

bool ZipVFS::indexSingleZip(const std::string& rootPath, const std::string& zipPath,
    const std::string& virtualPrefix, int nestingDepth) {
    const int MAX_NESTING_DEPTH = 10;
    if (nestingDepth > MAX_NESTING_DEPTH) {
        std::cerr << "ZipVFS: max nesting depth exceeded for: " << zipPath << "\n";
        return false;
    }

    zip_t* za = openZipFromPathOrMemory(zipPath);
    if (!za) {
        std::cerr << "ZipVFS: failed to open zip: " << zipPath << "\n";
        return false;
    }

    // Calculate the virtual path prefix for entries in this zip
    std::string entryPrefix;

    if (nestingDepth == 0) {
        // Top-level zip: mount in a directory named after the zip file (without extension)
        fs::path zipFilePath(zipPath);
        std::string zipFileName = zipFilePath.stem().string();  // filename without extension

        // Get relative path of zip file to rootPath
        fs::path zipFsPath = zipFilePath.parent_path();
        fs::path rootFsPath = fs::path(rootPath);
        fs::path mountPath;

        try {
            mountPath = fs::relative(zipFsPath, rootFsPath);
        }
        catch (...) {
            mountPath.clear();
        }

        // Construct: rootPath + relative_dir + zip_name_without_ext + /
        entryPrefix = rootPath;

        std::string relativeDir = mountPath.generic_string();
        if (!relativeDir.empty() && relativeDir != ".") {
            if (relativeDir.back() != '/') relativeDir += '/';
            entryPrefix += relativeDir;
        }

        //entryPrefix += zipFileName + "/";
    }
    else {
        // Nested zip: use provided virtual prefix
        entryPrefix = virtualPrefix;
    }

    int total = zip_entries_total(za);
    if (total < 0) total = 0;

    std::vector<NestedZipEntry> nestedZips;

    for (int i = 0; i < total; ++i) {
        if (zip_entry_openbyindex(za, i) < 0) {
            zip_close(za);
            return false;
        }

        const char* name = zip_entry_name(za);
        if (!name) {
            zip_entry_close(za);
            continue;
        }

        std::string entryName(name);

        // Construct full virtual path using the calculated prefix
        std::string vpath = entryPrefix + entryName;

        if (vpath.rfind("./", 0) == 0)
            vpath = vpath.substr(2);

        // Check if this entry is a nested zip file
        bool isNested = isZipFile(entryName);

        if (isNested) {
            // Store for later extraction and indexing
            NestedZipEntry nested;
            nested.virtualPath = vpath;
            nested.parentZipPath = zipPath;
            nested.entryIndex = i;
            nested.entryName = entryName;
            nestedZips.push_back(nested);

            std::cout << "ZipVFS: Found nested zip: " << vpath
                << " (depth " << nestingDepth << ")\n";
        }

        // Add ALL entries to index (including nested zips themselves)
        SourceEntry src;
        src.zipPath = zipPath;
        src.entryIndex = i;
        src.uncompressedSize = static_cast<uint64_t>(zip_entry_size(za));
        src.mtime = 0;
        src.isNested = (nestingDepth > 0);

        m_index[vpath].push_back(src);

        // Sort by zipPath (alphabetically descending for priority)
        auto& vec = m_index[vpath];
        std::sort(vec.begin(), vec.end(),
            [](const SourceEntry& a, const SourceEntry& b) {
                return a.zipPath > b.zipPath;
            });

        zip_entry_close(za);
    }

    zip_close(za);

    // Process nested zips AFTER closing parent zip
    for (const auto& nested : nestedZips) {
        if (!extractAndIndexNestedZip(rootPath, nested, nestingDepth + 1)) {
            std::cerr << "ZipVFS: failed to process nested zip: "
                << nested.entryName << " in " << nested.parentZipPath << "\n";
            return false;
        }
    }

    return true;
}

bool ZipVFS::extractAndIndexNestedZip(const std::string& rootPath,
    const NestedZipEntry& nested,
    int currentDepth) {
    std::cout << "ZipVFS: Extracting nested zip: " << nested.virtualPath << "\n";

    // Open parent zip (could be file or memory)
    zip_t* za = openZipFromPathOrMemory(nested.parentZipPath);
    if (!za) {
        std::cerr << "ZipVFS: Failed to reopen parent zip: " << nested.parentZipPath << "\n";
        return false;
    }

    if (zip_entry_openbyindex(za, nested.entryIndex) < 0) {
        zip_close(za);
        std::cerr << "ZipVFS: Failed to open nested entry at index " << nested.entryIndex << "\n";
        return false;
    }

    // Read nested zip data into memory
    void* buf = nullptr;
    size_t bufsize = 0;
    int r = zip_entry_read(za, &buf, &bufsize);

    zip_entry_close(za);
    zip_close(za);

    if (r < 0 || buf == nullptr || bufsize == 0) {
        if (buf) free(buf);
        std::cerr << "ZipVFS: Failed to read nested zip data\n";
        return false;
    }

    // Store the nested zip data in memory
    std::vector<uint8_t> zipData(static_cast<uint8_t*>(buf),
        static_cast<uint8_t*>(buf) + bufsize);
    free(buf);

    // Generate unique identifier for this nested zip
    std::string nestedZipId = "memory://" + std::to_string(m_nestedZipData.size()) +
        "/" + nested.entryName;

    m_nestedZipData[nestedZipId] = std::move(zipData);

    std::cout << "ZipVFS: Stored nested zip in memory: " << nestedZipId
        << " (" << m_nestedZipData[nestedZipId].size() << " bytes)\n";

    // Calculate virtual prefix for nested content
    // Use the PARENT DIRECTORY of the nested zip file (unpack in place)
    std::string nestedPrefix = nested.virtualPath;

    // Get parent directory by finding last slash
    size_t lastSlash = nestedPrefix.rfind('/');
    if (lastSlash != std::string::npos) {
        nestedPrefix = nestedPrefix.substr(0, lastSlash + 1);  // Keep trailing slash
    }
    else {
        nestedPrefix = "";  // Root directory
    }

    std::cout << "ZipVFS: Indexing nested zip with prefix: " << nestedPrefix << "\n";

    // Recursively index the nested zip (now stored in memory)
    return indexSingleZip(rootPath, nestedZipId, nestedPrefix, currentDepth);
}

zip_t* ZipVFS::openZipFromPathOrMemory(const std::string& zipPath) {
    // Check if this is a memory-based zip
    if (zipPath.rfind("memory://", 0) == 0) {
        auto it = m_nestedZipData.find(zipPath);
        if (it == m_nestedZipData.end()) {
            std::cerr << "ZipVFS: Memory zip not found: " << zipPath << "\n";
            return nullptr;
        }

        // Open from memory using zip_stream_open
        const auto& data = it->second;
        zip_t* za = zip_stream_open(reinterpret_cast<const char*>(data.data()),
            data.size(), 0, 'r');
        if (!za) {
            std::cerr << "ZipVFS: zip_stream_open failed for: " << zipPath << "\n";
        }
        return za;
    }
    else {
        // Regular file-based zip
        return zip_open(zipPath.c_str(), 0, 'r');
    }
}

std::optional<std::vector<uint8_t>> ZipVFS::readFromZip(const SourceEntry& src) {
    zip_t* za = openZipFromPathOrMemory(src.zipPath);
    if (!za) {
        std::cerr << "ZipVFS: readFromZip failed to open: " << src.zipPath << "\n";
        return std::nullopt;
    }

    if (zip_entry_openbyindex(za, src.entryIndex) < 0) {
        zip_close(za);
        std::cerr << "ZipVFS: readFromZip zip_entry_openbyindex failed for "
            << src.zipPath << " index " << src.entryIndex << "\n";
        return std::nullopt;
    }

    void* buf = nullptr;
    size_t bufsize = 0;
    int r = zip_entry_read(za, &buf, &bufsize);

    if (r < 0 || buf == nullptr || bufsize == 0) {
        zip_entry_close(za);
        zip_close(za);
        if (buf) free(buf);
        return std::nullopt;
    }

    std::vector<uint8_t> out;
    out.resize(bufsize);
    memcpy(out.data(), buf, bufsize);
    free(buf);

    zip_entry_close(za);
    zip_close(za);

    return out;
}

std::optional<std::string> ZipVFS::ReadFileAsText(const std::string& virtualPath) {
    auto binOpt = ReadFileAsBinary(virtualPath);
    if (!binOpt) return std::nullopt;
    return std::string(binOpt->begin(), binOpt->end());
}

std::optional<std::vector<uint8_t>> ZipVFS::ReadFileAsBinary(const std::string& virtualPath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_index.find(virtualPath);
    if (it == m_index.end()) {
        //std::cerr << "ZipVFS: File not found in index: " << virtualPath << "\n";
        return std::nullopt;
    }

    if (virtualPath == "GameData/textures/generic/grass.png") {
        printf("Debug breakpoint location\n");
    }

    const auto& sources = it->second;
    if (sources.empty()) return std::nullopt;

    // Try reading from sources in priority order
    for (const auto& src : sources) {
        auto readRes = readFromZip(src);
        if (readRes) {
            std::cout << "ZipVFS: Successfully read " << virtualPath
                << " from " << src.zipPath << "\n";
            return readRes;
        }
    }

    std::cerr << "ZipVFS: Failed to read from any source for: " << virtualPath << "\n";
    return std::nullopt;
}

std::vector<std::string> ZipVFS::GetFilesInPath(const std::string& virtualDir) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> out;

    std::string prefix = virtualDir;
    if (!prefix.empty() && prefix.back() != '/')
        prefix += '/';

    std::unordered_set<std::string> foundNames;

    for (const auto& kv : m_index) {
        const std::string& fullPath = kv.first;
        if (fullPath.rfind(prefix, 0) != 0) continue;

        std::string rest = fullPath.substr(prefix.size());
        if (rest.empty()) continue;

        auto pos = rest.find('/');
        std::string entryName;
        if (pos == std::string::npos) {
            entryName = rest;
        }
        else {
            entryName = rest.substr(0, pos);
        }

        if (foundNames.insert(entryName).second) {
            out.push_back(entryName);
        }
    }

    return out;
}

uint32_t ZipVFS::GetFileModificationTime(const std::string& virtualPath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_index.find(virtualPath);
    if (it == m_index.end()) return 0;
    if (it->second.empty()) return 0;

    return it->second.front().mtime;
}