#include "ZipVFS.h"
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <cassert>
#include <unordered_set>
#include <chrono>


#include "../Compression/zip/zip.h"

namespace fs = std::filesystem;

ZipVFS& ZipVFS::Instance() {
    static ZipVFS inst;
    return inst;
}

bool ZipVFS::Init(const std::string& rootPath) 
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_index.clear();
    m_zipFiles.clear();

    std::vector<std::string> foundZips;
    if (!scanForZips(rootPath, foundZips)) {
        std::cerr << "ZipVFS: failed scanning for zips under: " << rootPath << "\n";
        return false;
    }

    // sort alphabetical (full path) - std::string lexicographic which is what you requested
    std::sort(foundZips.begin(), foundZips.end());
    m_zipFiles = foundZips;

    // index each zip (upfront indexing)
    for (const auto& zp : m_zipFiles) {
        if (!indexSingleZip(rootPath, zp))
        {
            std::cerr << "ZipVFS: failed indexing zip: " << zp << "\n";
            // per requirement: fail initialization on corrupt zip
            m_index.clear();
            m_zipFiles.clear();
            return false;
        }
    }

    return true;
}

void ZipVFS::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_index.clear();
    m_zipFiles.clear();
}

bool ZipVFS::scanForZips(const std::string& rootPath, std::vector<std::string>& outZipPaths) {
    try {
        if (!fs::exists(rootPath)) return true; // nothing to index, but not an error
        for (auto const& dirEntry : fs::recursive_directory_iterator(rootPath)) {
            if (!dirEntry.is_regular_file()) continue;
            auto ext = dirEntry.path().extension().string();
            std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) { return std::tolower(c); });
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

bool ZipVFS::indexSingleZip(const std::string& rootPath,
    const std::string& zipPath)
{
    zip_t* za = zip_open(zipPath.c_str(), 0, 'r');
    if (!za) return false;

    fs::path zipFsPath = fs::path(zipPath).parent_path();
    fs::path rootFsPath = fs::path(rootPath);

    fs::path mountPath;
    try {
        mountPath = fs::relative(zipFsPath, rootFsPath);
    }
    catch (...) {
        mountPath.clear();
    }

    std::string mountPrefix = mountPath.generic_string();
    if (!mountPrefix.empty() && mountPrefix.back() != '/')
        mountPrefix += '/';

    int total = zip_entries_total(za);
    if (total < 0) total = 0;

    for (int i = 0; i < total; ++i)
    {
        if (zip_entry_openbyindex(za, i) < 0)
        {
            zip_close(za);
            return false;
        }

        const char* name = zip_entry_name(za);
        if (!name)
        {
            zip_entry_close(za);
            continue;
        }

        std::string vpath = rootPath + name;

        if (vpath.rfind("./", 0) == 0)
            vpath = vpath.substr(2);

        SourceEntry src;
        src.zipPath = zipPath;
        src.entryIndex = i;
        src.uncompressedSize = static_cast<uint64_t>(zip_entry_size(za));
        src.mtime = 0;

        if (vpath == "GameData/textures/generic/grass.png")
        {
			printf("Debug breakpoint location\n");
        }

        m_index[vpath].push_back(src);


        // Sort the vector alphabetically by zipPath
        auto& vec = m_index[vpath];
        std::sort(vec.begin(), vec.end(), [](const SourceEntry& a, const SourceEntry& b) {
            return a.zipPath > b.zipPath; // sort by zipPath
            });


        zip_entry_close(za);
    }

    zip_close(za);
    return true;
}



std::optional<std::vector<uint8_t>> ZipVFS::readFromZip(const SourceEntry& src) {
    // open zip file
    struct zip_t* za = zip_open(src.zipPath.c_str(), 0, 'r');
    if (!za) {
        std::cerr << "ZipVFS: readFromZip zip_open failed for " << src.zipPath << "\n";
        return std::nullopt;
    }

    // open entry by index
    if (zip_entry_openbyindex(za, src.entryIndex) < 0) {
        zip_close(za);
        std::cerr << "ZipVFS: readFromZip zip_entry_openbyindex failed for " << src.zipPath << " index " << src.entryIndex << "\n";
        return std::nullopt;
    }

    // read into allocated buffer via zip_entry_read(...) as per README example
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

// Read text (returns empty optional if not found)
std::optional<std::string> ZipVFS::ReadFileAsText(const std::string& virtualPath) {
    auto binOpt = ReadFileAsBinary(virtualPath);
    if (!binOpt) return std::nullopt;
    // Assume file is text (not doing encoding conversions)
    return std::string(binOpt->begin(), binOpt->end());
}

std::optional<std::vector<uint8_t>> ZipVFS::ReadFileAsBinary(const std::string& virtualPath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_index.find(virtualPath);
    if (it == m_index.end()) return std::nullopt;


    if (virtualPath == "GameData/textures/generic/grass.png")
    {
        printf("Debug breakpoint location\n");
    }

    // vector of sources is ordered by insertion (we added zipPaths in alphabetical order),
    // so first element is highest-priority zip among zips. We must still respect disk-over-zip
    // at caller level (FileSystem will check disk first).
    const auto& sources = it->second;
    if (sources.empty()) return std::nullopt;

    // Try reading from the first source (highest priority)
    for (const auto& src : sources) {
        auto readRes = readFromZip(src);
        if (readRes) return readRes; // return first successful read
    }
    return std::nullopt;
}

std::vector<std::string> ZipVFS::GetFilesInPath(const std::string& virtualDir) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<std::string> out;

    // Ensure directory path ends with '/'
    std::string prefix = virtualDir;
    if (!prefix.empty() && prefix.back() != '/') prefix += '/';

    std::unordered_set<std::string> foundNames;
    for (const auto& kv : m_index) {
        const std::string& fullPath = kv.first;
        if (fullPath.rfind(prefix, 0) != 0) continue; // not in this folder

        // extract immediate child name (no subdir part)
        std::string rest = fullPath.substr(prefix.size());
        if (rest.empty()) continue;
        auto pos = rest.find('/');
        std::string entryName;
        if (pos == std::string::npos) {
            entryName = rest; // file in this dir
        }
        else {
            entryName = rest.substr(0, pos); // a subdirectory
        }

        if (foundNames.insert(entryName).second) {
            out.push_back(entryName);
        }
    }

    // Note: caller (FileSystem) must merge with disk contents; we only return zip-derived names here.
    return out;
}

uint32_t ZipVFS::GetFileModificationTime(const std::string& virtualPath) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_index.find(virtualPath);
    if (it == m_index.end()) return 0;
    // per zip ordering the first source is highest priority among zips
    if (it->second.empty()) return 0;
    return it->second.front().mtime;
}
