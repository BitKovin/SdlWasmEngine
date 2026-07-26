#include "NativeFileSystem.h"
#include <SDL2/SDL.h>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

// Directory listing, IsDirectory(), and file-modification-time all still go
// through std::filesystem -- SDL2 has no cross-platform API for any of
// these (SDL_RWops is read/write only). Note that on Android this means
// those three calls only work against real filesystem paths (internal /
// external storage), not against assets packaged inside the APK; asset
// entries aren't stat-able or enumerable the way a normal directory is.
// ReadFile/ReadFileBinary/WriteSaveFile/WriteSaveFileBinary below go
// through SDL_RWops instead, which is what lets them reach into APK
// assets (via AAssetManager under the hood) and Emscripten's virtual FS
// the same way a desktop path would work.

bool NativeFileSystem::Init() { return true; }
void NativeFileSystem::Shutdown() {}

std::optional<std::string> NativeFileSystem::ReadFile(const std::string& path) {
    SDL_RWops* rw = SDL_RWFromFile(path.c_str(), "rb");
    if (!rw) {
        SDL_Log("NativeFileSystem::ReadFile: couldn't open '%s': %s", path.c_str(), SDL_GetError());
        return std::nullopt;
    }

    Sint64 size = SDL_RWsize(rw);
    if (size < 0) {
        SDL_RWclose(rw);
        return std::nullopt;
    }

    std::string result(static_cast<size_t>(size), '\0');
    size_t totalRead = 0;
    while (totalRead < result.size()) {
        size_t n = SDL_RWread(rw, result.data() + totalRead, 1, result.size() - totalRead);
        if (n == 0) break; // EOF or read error -- SDL_RWread doesn't distinguish
        totalRead += n;
    }
    SDL_RWclose(rw);

    if (totalRead != result.size()) return std::nullopt;
    return result;
}

std::optional<std::vector<uint8_t>> NativeFileSystem::ReadFileBinary(const std::string& path) {
    SDL_RWops* rw = SDL_RWFromFile(path.c_str(), "rb");
    if (!rw) {
        SDL_Log("NativeFileSystem::ReadFileBinary: couldn't open '%s': %s", path.c_str(), SDL_GetError());
        return std::nullopt;
    }

    Sint64 size = SDL_RWsize(rw);
    if (size < 0) {
        SDL_RWclose(rw);
        return std::nullopt;
    }

    std::vector<uint8_t> data(static_cast<size_t>(size));
    size_t totalRead = 0;
    while (totalRead < data.size()) {
        size_t n = SDL_RWread(rw, data.data() + totalRead, 1, data.size() - totalRead);
        if (n == 0) break;
        totalRead += n;
    }
    SDL_RWclose(rw);

    if (totalRead != data.size()) return std::nullopt;
    return data;
}

std::vector<std::string> NativeFileSystem::GetFilesInPath(const std::string& path) {
    std::vector<std::string> result;
    try {
        if (fs::exists(path) && fs::is_directory(path)) {
            for (const auto& entry : fs::directory_iterator(path)) {
                result.push_back(entry.path().filename().string());
            }
        }
    }
    catch (...) {}
    return result;
}

bool NativeFileSystem::IsDirectory(const std::string& path) {
    try { return fs::exists(path) && fs::is_directory(path); }
    catch (...) { return false; }
}

uint32_t NativeFileSystem::GetFileModificationTime(const std::string& path) {
    try {
        if (fs::exists(path) && fs::is_regular_file(path)) {
            auto ftime = fs::last_write_time(path);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
            );
            return static_cast<uint32_t>(std::chrono::system_clock::to_time_t(sctp));
        }
    }
    catch (...) {}
    return 0;
}

bool NativeFileSystem::WriteSaveFile(const std::string& path, const std::string& content) {
    try { fs::create_directories(fs::path(path).parent_path()); }
    catch (...) { return false; }

    SDL_RWops* rw = SDL_RWFromFile(path.c_str(), "wb");
    if (!rw) {
        SDL_Log("NativeFileSystem::WriteSaveFile: couldn't open '%s': %s", path.c_str(), SDL_GetError());
        return false;
    }

    size_t written = SDL_RWwrite(rw, content.data(), 1, content.size());
    SDL_RWclose(rw);
    return written == content.size();
}

bool NativeFileSystem::WriteSaveFileBinary(const std::string& path, const std::vector<uint8_t>& data) {
    try { fs::create_directories(fs::path(path).parent_path()); }
    catch (...) { return false; }

    SDL_RWops* rw = SDL_RWFromFile(path.c_str(), "wb");
    if (!rw) {
        SDL_Log("NativeFileSystem::WriteSaveFileBinary: couldn't open '%s': %s", path.c_str(), SDL_GetError());
        return false;
    }

    size_t written = SDL_RWwrite(rw, data.data(), 1, data.size());
    SDL_RWclose(rw);
    return written == data.size();
}

std::optional<std::string> NativeFileSystem::ReadSaveFile(const std::string& path) { return ReadFile(path); }
std::optional<std::vector<uint8_t>> NativeFileSystem::ReadSaveFileBinary(const std::string& path) { return ReadFileBinary(path); }
std::string NativeFileSystem::GetPhysicalPath(const std::string& path) { return path; }