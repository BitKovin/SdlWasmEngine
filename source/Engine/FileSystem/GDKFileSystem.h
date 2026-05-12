#pragma once
#include "NativeFileSystem.h"

// GDKFileSystem — XGameSave-backed save data for Microsoft GDK (Xbox / Gaming Desktop).
//
// Drop-in replacement for NativeFileSystem. On non-GDK builds it transparently
// falls back to NativeFileSystem.
//
// Design
// ------
// All save data lives in a single XGameSave container ("savedata"). The full
// relative path is encoded directly into each blob name by replacing every '/'
// with the escape sequence "~~":
//
//   SaveData/Preferences/videoSettings.json
//       → blob  "Preferences~~videoSettings.json"
//
//   SaveData/Profiles/profile1/saves/quicksave.sav
//       → blob  "Profiles~~profile1~~saves~~quicksave.sav"
//
// GetFilesInPath enumerates every blob in the container, decodes the name back
// to a path, and returns the immediate children of the requested directory.
// Because the path lives in the blob name there is no separate manifest —
// files can be added, removed, or renamed externally (Xbox Device Portal, GDK
// tools) and the filesystem will reflect the change automatically.
//
// Usage
// -----
//   auto fs = std::make_unique<GDKFileSystem>("00000000-0000-0000-0000-000000000000");
//   fs->Init();
//   fs->WriteSaveFile("SaveData/Profiles/profile1/saves/quicksave.sav", data);
//   auto names = fs->GetFilesInPath("SaveData/Profiles/profile1/saves/");
//   // → ["quicksave.sav", …]
//   fs->Shutdown();

#if defined(PLATFORM_GDK) || defined(__GDK__)
#  define GDK_GAMESAVE_SUPPORTED 1
#  include <windows.h>
#  include <XGameSave.h>
#  include <XUser.h>
#  include <XTaskQueue.h>
#endif

#include <optional>
#include <string>
#include <vector>
#include <cstdint>

class GDKFileSystem : public NativeFileSystem {
public:
    // scid — Service Configuration ID from MicrosoftGame.config.
    //        Format: "00000000-0000-0000-0000-000000000000"
    explicit GDKFileSystem(const char* scid);
    ~GDKFileSystem() override;

    bool Init() override;
    void Shutdown() override;

    bool WriteSaveFile(const std::string& path, const std::string& content) override;
    std::optional<std::string> ReadSaveFile(const std::string& path) override;
    bool WriteSaveFileBinary(const std::string& path, const std::vector<uint8_t>& data) override;
    std::optional<std::vector<uint8_t>> ReadSaveFileBinary(const std::string& path) override;

    // Enumerates immediate children of a "SaveData/…" virtual directory by
    // scanning blob names in the container. Falls back to NativeFileSystem
    // for all other paths.
    std::vector<std::string> GetFilesInPath(const std::string& path) override;

private:
#if defined(GDK_GAMESAVE_SUPPORTED)

    // -----------------------------------------------------------------------
    // Path encoding
    //   '/' in a relative path  ↔  "~~" in a blob name
    // -----------------------------------------------------------------------
    static std::string PathToBlob(const std::string& relPath);  // encode
    static std::string BlobToPath(const std::string& blobName); // decode

    // Strips the "SaveData/" prefix when present, returning the bare relative path.
    static std::string StripSavePrefix(const std::string& path);

    // -----------------------------------------------------------------------
    // Blob I/O
    // -----------------------------------------------------------------------
    bool WriteBlobInternal(const std::string& blobName,
        const void* data,
        uint32_t           dataSize);

    std::optional<std::vector<uint8_t>> ReadBlobInternal(const std::string& blobName);

    // -----------------------------------------------------------------------
    // Constants
    // -----------------------------------------------------------------------
    static constexpr const char* kSaveDataPrefix = "SaveData/";

    // Single container that holds every save blob.
    static constexpr const char* kSaveContainer = "savedata";

    // Replaces '/' inside blob names so directory structure is encoded in name.
    static constexpr const char* kDirSeparator = "~~";

    // -----------------------------------------------------------------------
    // State
    // -----------------------------------------------------------------------
    std::string             m_scid;
    XUserHandle             m_user = nullptr;
    XGameSaveProviderHandle m_provider = nullptr;
    XTaskQueueHandle        m_queue = nullptr;

#else
    std::string m_scid;
#endif
};