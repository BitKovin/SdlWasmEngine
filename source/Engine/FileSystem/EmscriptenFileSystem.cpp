#include "EmscriptenFileSystem.h"
#include <fstream>
#include <sstream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(void, SetupPersistentSystemJS, (), {
    try {
        // Create the root mount point
        if (!FS.analyzePath('/saves').exists) {
            FS.mkdir('/saves');
        }
        // Mount IndexedDB
        FS.mount(IDBFS, {}, '/saves');

        // Sync from IDB to Memory at startup
        FS.syncfs(true, function(err) {
            if (err) console.error("Initial sync error:", err);
        });
    }
 catch (e) {
  console.error("SetupPersistentSystemJS failed:", e);
}
    });

EM_JS(void, WriteFileJS, (const char* relPath, const char* content), {
    const path = UTF8ToString(relPath);
    const data = UTF8ToString(content);

    try {
        // Handle nested directories: "saves/subdir/another/file.txt"
        const parts = path.split('/');
        let current = '';
        // Loop through parts, but skip the last one (the filename)
        for (let i = 0; i < parts.length - 1; i++) {
            current += (i ? '/' : '') + parts[i];
            if (current.length > 0 && !FS.analyzePath(current).exists) {
                FS.mkdir(current);
            }
        }

        FS.writeFile(path, data);

        // Commit to IndexedDB
        FS.syncfs(false, function(err) {
            if (err) console.error("Write sync error:", err);
        });
    }
 catch (e) {
  console.error("WriteFileJS failed:", e);
}
    });

// Base64 helpers for Binary -> Text conversion
static const std::string b64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

static std::string Base64Encode(const std::vector<uint8_t>& in) {
    std::string out;
    int val = 0, valb = -6;
    for (uint8_t c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(b64chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) out.push_back(b64chars[((val << 8) >> (valb + 8)) & 0x3F]);
    while (out.size() % 4) out.push_back('=');
    return out;
}

static std::vector<uint8_t> Base64Decode(const std::string& in) {
    std::vector<uint8_t> out;
    std::vector<int> T(256, -1);
    for (int i = 0; i < 64; i++) T[b64chars[i]] = i;
    int val = 0, valb = -8;
    for (char c : in) {
        if (T[c] == -1) break;
        val = (val << 6) + T[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(static_cast<uint8_t>((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}
#endif

EmscriptenFileSystem::EmscriptenFileSystem()
{
    Init();
}

bool EmscriptenFileSystem::Init() {
#ifdef __EMSCRIPTEN__
    SetupPersistentSystemJS();
#endif
    return true;
}

bool EmscriptenFileSystem::WriteSaveFile(const std::string& path, const std::string& content) {
#ifdef __EMSCRIPTEN__
    // Prepend the persistent root
    std::string fullPath = "/saves/" + path;
    WriteFileJS(fullPath.c_str(), content.c_str());
    return true;
#else
    return NativeFileSystem::WriteSaveFile(path, content);
#endif
}

std::optional<std::string> EmscriptenFileSystem::ReadSaveFile(const std::string& path) {
#ifdef __EMSCRIPTEN__
    std::string fullPath = "/saves/" + path;
    std::ifstream file(fullPath);
    if (file.is_open()) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    return std::nullopt;
#else
    return NativeFileSystem::ReadSaveFile(path);
#endif
}

bool EmscriptenFileSystem::WriteSaveFileBinary(const std::string& path, const std::vector<uint8_t>& data) {
#ifdef __EMSCRIPTEN__
    // Convert to text and save
    return WriteSaveFile(path, Base64Encode(data));
#else
    return NativeFileSystem::WriteSaveFileBinary(path, data);
#endif
}

std::optional<std::vector<uint8_t>> EmscriptenFileSystem::ReadSaveFileBinary(const std::string& path) {
#ifdef __EMSCRIPTEN__
    auto text = ReadSaveFile(path);
    if (text) {
        return Base64Decode(*text);
    }
    return std::nullopt;
#else
    return NativeFileSystem::ReadSaveFileBinary(path);
#endif
}

std::vector<std::string> EmscriptenFileSystem::GetFilesInPath(const std::string& path) {
#ifdef __EMSCRIPTEN__
    // If the logical path starts with "SaveData/", map it to the IndexedDB mount point
    // exactly the same way WriteSaveFile/ReadSaveFile do (no stripping of "SaveData/")
    if (path.rfind("SaveData/", 0) == 0) {
        std::string fullPath = "/saves/" + path;
        return NativeFileSystem::GetFilesInPath(fullPath);
    }
    // All other paths (assets, etc.) fall back to normal MEMFS behavior
    return NativeFileSystem::GetFilesInPath(path);
#else
    return NativeFileSystem::GetFilesInPath(path);
#endif
}