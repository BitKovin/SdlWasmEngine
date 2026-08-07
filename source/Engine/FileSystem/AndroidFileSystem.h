#pragma once
#include "NativeFileSystem.h"
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class AndroidFileSystem : public NativeFileSystem {
public:
    AndroidFileSystem();
    ~AndroidFileSystem() override = default;

    bool Init() override;

    // NativeFileSystem::ReadFile / ReadFileBinary / WriteSaveFile /
    // WriteSaveFileBinary already go through SDL_RWops, and SDL_RWFromFile
    // on Android already resolves relative paths against the APK's assets
    // (via AAssetManager under the hood) and absolute paths against real
    // storage -- so those four need no override here. The three below exist
    // because SDL has no API for any of them at all. They now read from
    // GameData/manifest.json (written by finalize_gamedata.py) whenever the
    // queried path falls under the manifest's root, since that's a
    // complete, reliable, build-time index -- including files folded into
    // a .zip by the finalizer, which AAssetManager could never see at all.
    // Anything outside the manifest's root falls back to best-effort
    // AAssetManager enumeration, same as before.
    std::vector<std::string> GetFilesInPath(const std::string& path) override;
    bool IsDirectory(const std::string& path) override;
    uint32_t GetFileModificationTime(const std::string& path) override;

    // NativeFileSystem::GetPhysicalPath() unconditionally echoes the input
    // path back, which is correct for a real filesystem path but wrong for
    // an asset-relative path: those live packed inside the APK with no
    // fopen()-able location on disk, reachable only via AAssetManager (the
    // same reason SDL_RWFromFile special-cases them). Overridden here so
    // callers like ZipVFS -- which treat a non-empty GetPhysicalPath() as a
    // green light to zip_open() the path directly against the real
    // filesystem -- correctly fall back to reading the bytes into memory
    // instead.
    std::string GetPhysicalPath(const std::string& path) override;

    bool WriteSaveFile(const std::string& path, const std::string& content);
    bool WriteSaveFileBinary(const std::string& path, const std::vector<uint8_t>& data);

    std::optional<std::string> ReadSaveFile(const std::string& path);
    std::optional<std::vector<uint8_t>> ReadSaveFileBinary(const std::string& path);

private:
    // Mirrors the exact rule SDL_RWFromFile itself uses on Android: a
    // leading '/' means "real filesystem path" (internal/external storage),
    // anything else means "asset-relative path" (packaged inside the APK).
    static bool IsAssetPath(const std::string& path);
    bool IsSaveDataPath(const std::string& path);

    static std::string GetSaveDataRoot();
    static std::optional<std::string> ToSaveDataPhysicalPath(const std::string& path);

    // Non-static: needs *this* as the implicit object to call the base
    // class's NativeFileSystem::GetFileModificationTime through, since that
    // is itself a non-static member function. Only meaningfully defined
    // under __ANDROID__ in the .cpp.
    uint32_t GetApkModTime();

    // --- manifest.json support -------------------------------------------

    struct ManifestFileEntry {
        uint64_t size = 0;
        uint32_t mtime = 0;
        std::string archive; // non-empty if this file lives inside a .zip
    };

    // Parses GameData/manifest.json into the maps below. Returns false (and
    // leaves the manifest disabled) if it's missing or malformed -- callers
    // then fall back to AAssetManager enumeration for everything, same as
    // before this manifest existed.
    bool LoadManifest();

    // If `path` falls under the manifest's root (e.g. "GameData/"), returns
    // the path relative to that root (with the prefix stripped, "" for the
    // root itself). Returns std::nullopt if it doesn't -- a different
    // mount, or the manifest failed to load -- meaning callers should fall
    // back instead of trusting an empty manifest lookup as "not found".
    std::optional<std::string> ToManifestRelative(const std::string& path) const;

    bool m_manifestLoaded = false;
    std::string m_manifestRootPrefix; // e.g. "GameData/", trailing slash included
    std::unordered_set<std::string> m_manifestDirs;
    std::unordered_map<std::string, ManifestFileEntry> m_manifestFiles;
    std::unordered_map<std::string, std::vector<std::string>> m_manifestDirChildren;
};
