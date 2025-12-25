// FileSystem.cpp
#include "FileSystem.h"
#include <stdexcept>
#include <SDL2/SDL.h>
#include <sstream>

#include "../Logger.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/emscripten.h>



EM_JS(void, WriteFileJS, (const char* relPath, const char* content), {
    const rel = UTF8ToString(relPath);
    const data = UTF8ToString(content);

    // build each sub‑directory under /save
    const parts = rel.split('/');
    let cur = '/save';
    for (let i = 0; i < parts.length - 1; ++i) {
        cur += '/' + parts[i];
        try { FS.mkdir(cur); }
 catch (e) { /* already exists */ }
}

    // now write the file
    const full = '/save/' + rel;
    FS.writeFile(full, data);

    // persist to IndexedDB
    FS.syncfs(false, function(err) {
        if (err) console.error('IDBFS write sync failed:', err);
    });
});


EM_JS(char*, ReadFileJS, (const char* relPath), {
  try {
    const path = '/save/' + UTF8ToString(relPath);
    const data = FS.readFile(path, { encoding: 'utf8' });
    const len = lengthBytesUTF8(data) + 1;
    const buf = _malloc(len);
    stringToUTF8(data, buf, len);
    return buf;
  }
catch (e) {
return 0;
}
    });

EM_JS(void, MountPersistentFS, (), {
    FS.mkdir('/save');
    FS.mount(IDBFS, {}, '/save');
    FS.syncfs(true, function(err) {
        if (err) console.error('IDBFS initial load failed:', err);
    });
    });


#else
#include <fstream>
#include <sstream>
#include <filesystem>  // C++17
#endif

namespace FileSystemEngine {


    void Init()
    {
#ifdef __EMSCRIPTEN__
        MountPersistentFS();
#endif
    }

    static bool isSavePath(const std::string& p) {
        return p.rfind("SaveData/", 0) == 0;
    }

    // strips "SaveData/" prefix
    static std::string stripSavePrefix(const std::string& p) {
        return p.substr(strlen("SaveData/"));
    }

    std::vector<uint8_t> ReadFileBinary(const std::string& path) {
        if (isSavePath(path)) {
            // --- SaveData binary read on desktop ---
#ifdef __EMSCRIPTEN__
            throw std::runtime_error("Binary SaveData not supported on Emscripten");
#else
            auto real = path;// stripSavePrefix(path);
            std::ifstream ifs(real, std::ios::binary);
            if (!ifs) throw std::runtime_error("Cannot open save file: " + real);
            return { std::istreambuf_iterator<char>(ifs),
                     std::istreambuf_iterator<char>() };
#endif
        }
        else {
            // --- GameData: use SDL_RWFromFile everywhere ---
            SDL_RWops* rw = SDL_RWFromFile(path.c_str(), "rb");
            //if (!rw) throw std::runtime_error("Failed open: " + path + " – " + SDL_GetError());

            if (!rw)
            {
                return std::vector<uint8_t>();
            }

            Sint64 size = SDL_RWsize(rw);
            if (size < 0) { SDL_RWclose(rw); throw std::runtime_error("Bad file size: " + path); }
            std::vector<uint8_t> buf(size);
            Sint64 r = SDL_RWread(rw, buf.data(), 1, size);
            SDL_RWclose(rw);
            if (r != size) throw std::runtime_error("Short read: " + path);
            return buf;
        }
    }

    std::string ReadFile(const std::string& path) {
        if (isSavePath(path)) {
#ifdef __EMSCRIPTEN__
            // Emscripten: call JS, get malloc'd char*
            auto rel = path;// stripSavePrefix(path);
            char* c = ReadFileJS(rel.c_str());
            //if (!c) throw std::runtime_error("Failed to read save: " + rel);
            std::string s(c);
            free(c);

			Logger::Log("ReadFile (SaveData): " + s);

            return s;
#else

            std::filesystem::path real(path);

            // Open file (MSVC-safe version)
            std::FILE* file = nullptr;
#if defined(_MSC_VER)
            if (fopen_s(&file, real.string().c_str(), "rb") != 0 || !file)
                return "";
#else
            file = std::fopen(real.string().c_str(), "rb");
            if (!file)
                return "";
#endif

            std::fseek(file, 0, SEEK_END);
            long size = std::ftell(file);
            std::rewind(file);

            if (size <= 0) {
                std::fclose(file);
                return "";
            }

            std::string content;
            content.resize(static_cast<size_t>(size));

            size_t read = std::fread(content.data(), 1, content.size(), file);
            std::fclose(file);

            content.resize(read);
            return content;

#endif
        }
        else {
            // GameData text → just wrap ReadFileBinary
            auto bin = ReadFileBinary(path);
            return std::string(bin.begin(), bin.end());
        }
    }

#ifdef __EMSCRIPTEN__
    EM_JS(char*, ListFilesJS, (const char* dirPath), {
        const dir = UTF8ToString(dirPath);
        try {
            const entries = FS.readdir(dir).filter(f => f !== '.' && f !== '..');
            const files = entries.filter(f => {
                const p = dir.endsWith('/') ? dir + f : dir + '/' + f;
                return FS.isFile(FS.stat(p).mode);
            });
            const str = files.join('\n');
            const len = lengthBytesUTF8(str) + 1;
            const buf = _malloc(len);
            stringToUTF8(str, buf, len);
            return buf;
        }
     catch (e) {
      return 0;
  }
        });
#endif


    std::vector<std::string> GetFilesInPath(const std::string& path)
    {
        std::vector<std::string> files;
        std::string dir_path = path;
        if (!dir_path.empty() && dir_path.back() != '/') {
            dir_path += '/';
        }

        if (isSavePath(path)) {
            // Handle save data
#ifdef __EMSCRIPTEN__
            std::string em_dir = "/save/" + path;
            char* c = ListFilesJS(em_dir.c_str());
            if (c) {
                std::string list(c);
                free(c);
                std::stringstream ss(list);
                std::string file;
                while (std::getline(ss, file, '\n')) {
                    files.push_back(file);
                }
            }
#else
        // Desktop: use std::filesystem for save data
            try {
                for (const auto& entry : std::filesystem::directory_iterator(path)) {
                    if (entry.is_regular_file()) {
                        files.push_back(entry.path().filename().string());
                    }
                }
            }
            catch (...) {
                // Return empty on error
            }
#endif
        }
        else {
            // Handle game data
#ifdef __EMSCRIPTEN__
            std::string em_dir = path;
            if (!em_dir.empty() && em_dir[0] != '/') {
                em_dir = "/" + em_dir;
            }
            char* c = ListFilesJS(em_dir.c_str());
            if (c) {
                std::string list(c);
                free(c);
                std::stringstream ss(list);
                std::string file;
                while (std::getline(ss, file, '\n')) {
                    files.push_back(file);
                }
            }
#else
        // Desktop: use SDL_GetBasePath() + path for game data
            const char* base = SDL_GetBasePath();
            std::string full_path;
            if (base) {
                full_path = path;
                SDL_free((void*)base);
            }
            else {
                full_path = path;
            }
            try {
                for (const auto& entry : std::filesystem::directory_iterator(full_path)) {
                    if (entry.is_regular_file()) {
                        files.push_back(entry.path().filename().string());
                    }
                }
            }
            catch (...) {
                // Return empty on error
            }
#endif
        }
        return files;
    }


    bool FileSystemEngine::WriteFile(const std::string& path, const std::string& content) 
    {

#ifdef __EMSCRIPTEN__

        if (!isSavePath(path)) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "WriteFile only allowed under SaveData/: %s", path.c_str());
            return false;
        }
#endif
        auto rel = path;// stripSavePrefix(path);

#ifdef __EMSCRIPTEN__
        WriteFileJS(rel.c_str(), content.c_str());
        return true;
#else
        try {
            // === on‑demand directory creation starts here ===
            std::filesystem::path fsPath(rel);
            if (auto parent = fsPath.parent_path(); !parent.empty() && !std::filesystem::exists(parent)) {
                std::filesystem::create_directories(parent);
            }
            // === end directory logic ===

            std::ofstream ofs(rel, std::ios::binary);
            if (!ofs) return false;
            ofs.write(content.data(), content.size());
            return true;
        }
        catch (...) {
            return false;
        }
#endif
    }


} // namespace FileSystem
