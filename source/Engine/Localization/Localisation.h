// Localisation.h
//
// Static, header/cpp localisation system.
//
// - Loads one active language plus a fallback language, each assembled from
//   every .json file in a per-language folder (so a language can be split
//   across e.g. pause.json, story.json, ...).
// - LocalizeString() scans arbitrary text for ${key} tokens and replaces
//   them in place. It is the function you call every time you draw text, and
//   is written to be as cheap as possible when a string has no tokens at all.
// - CachedText is an optional helper for text that's evaluated every frame
//   but rarely changes (menu labels, HUD prompts): it only re-resolves when
//   the language changes or is hot-reloaded, so a steady-state call is just
//   an integer comparison.
//
// JSON format (one array of entries per file):
//
//   {
//     "entries": [
//       { "key": "PAUSE_TITLE",  "value": "Paused", "description": "Title at the top of the pause menu" },
//       { "key": "PAUSE_RESUME", "value": "Resume",  "description": "Button that resumes gameplay" }
//     ]
//   }
//
// Folder layout (one folder per language, any number of files inside it):
//
//   GameData/localisation/en/pause.json
//   GameData/localisation/en/story.json
//   GameData/localisation/fr/pause.json
//   GameData/localisation/fr/story.json
//
// Usage:
//
//   Localisation::Initialize("GameData/localisation", "en");
//   Localisation::SetLanguage("fr");                 // optional, runtime switch
//   std::string s = Localisation::LocalizeString("${PAUSE_TITLE}");
//
// Thread safety:
//   All of the static Localisation:: functions (Initialize, SetLanguage,
//   ReloadCurrentLanguage, GetString, LocalizeString, GetCurrentLanguage,
//   GetVersion, SetWarningHandler) may be called concurrently from any
//   number of threads.
//     - Reads (GetString, LocalizeString, GetCurrentLanguage) use a shared
//       (reader/writer) lock, so concurrent readers never block each other,
//       only a concurrent writer briefly blocks them. LocalizeString's
//       no-tokens-found fast path doesn't touch any lock at all.
//     - Writes (Initialize, SetLanguage, ReloadCurrentLanguage) do their
//       file reading and JSON parsing *before* taking the exclusive lock,
//       so a slow disk read on a loader thread never stalls a render thread
//       calling LocalizeString - the lock is only held for the brief,
//       in-memory swap-in of the freshly loaded data.
//   CachedText is a small per-instance cache and is NOT internally
//   synchronized - as with most lightweight helper objects, a single
//   instance is expected to be owned and used by one thread (e.g. the
//   widget that holds it). It's perfectly safe for different threads to
//   each own their own CachedText instances at the same time.
//   Your IO::ReadTextFile / IO::GetFilesInFolder replacements should be
//   safe to call concurrently from multiple threads (the standard library
//   versions here are; e.g. avoid a shared scratch buffer without your own
//   locking).
//
// Requirements:
//   - C++20 (heterogeneous/transparent unordered_map & unordered_set
//     lookup, so a ${key} token can be looked up as a std::string_view
//     without allocating a temporary std::string).
//   - nlohmann/json (https://github.com/nlohmann/json), included by the .cpp
//     only - this header has no JSON dependency.
//   - Link against your platform's threading library (e.g. -pthread with
//     GCC/Clang on Linux) since this uses <shared_mutex>/<mutex>.
//
// The two functions in the nested IO struct are the only place this system
// touches disk. Replace their .cpp bodies with your engine's own file
// reading / directory listing; keep the signatures the same.

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

// Set to 1 (e.g. in a debug/tools build) to keep each entry's "description"
// field in memory for editor/debug use. Off by default so shipped builds
// don't carry translator-facing notes they'll never use.
#ifndef LOCALIZATION_STORE_DESCRIPTIONS
#define LOCALIZATION_STORE_DESCRIPTIONS 0
#endif

class Localisation
{
public:
    Localisation() = delete; // static class - never instantiate

    // ------------------------------------------------------------------
    // Engine file IO. Replace the two bodies in Localisation.cpp with your
    // engine's own implementations; everything else in this file calls
    // through these two functions only. May be called from any thread.
    // ------------------------------------------------------------------
    struct IO
    {
        IO() = delete;

        // Reads the full contents of a text file. Returns false (and leaves
        // outContents unspecified) if the file couldn't be opened/read.
        static bool ReadTextFile(const std::string& filePath, std::string& outContents);

        // Returns the full paths of every regular file directly inside
        // folderPath whose extension matches (e.g. ".json"). Non-recursive.
        // Returns an empty vector if the folder doesn't exist or is empty.
        static std::vector<std::string> GetFilesInFolder(const std::string& folderPath,
                                                           const std::string& extension);
    };

    // ------------------------------------------------------------------
    // Setup
    // ------------------------------------------------------------------

    // Sets the root localisation folder (e.g. "GameData/localisation") and
    // the fallback language code (e.g. "en"), then loads the fallback
    // language from <rootFolder>/<defaultLanguageCode>/*.json. Call once at
    // startup before anything else. Returns false if the fallback language
    // failed to load (no folder, no files, or every file failed to parse).
    static bool Initialize(const std::string& rootFolder, const std::string& defaultLanguageCode);

    // Makes languageCode the active language, loading it from disk the
    // first time it's requested (switching back to an already-loaded
    // language later is instant - no re-read). The fallback language stays
    // loaded and is used for any key missing from the active language.
    // Returns false and leaves the previous language active if loading
    // fails.
    static bool SetLanguage(const std::string& languageCode);

    // Re-reads the active language, and the fallback language if different,
    // from disk. Meant for development hot-reload (e.g. bound to a debug
    // key or a file watcher) - not something to call every frame.
    static bool ReloadCurrentLanguage();

    static std::string GetCurrentLanguage();

    // Bumped once on every Initialize/SetLanguage/ReloadCurrentLanguage
    // call. Lock-free (an atomic counter), so CachedText's steady-state
    // check - and any caching you build yourself on top of LocalizeString -
    // costs a single atomic load with no lock.
    static uint32_t GetVersion();

    // ------------------------------------------------------------------
    // Lookup
    // ------------------------------------------------------------------

    // Looks up a single key directly: active language, then fallback
    // language, then a visible "<key>" placeholder if it's nowhere to be
    // found (and reports it once via the warning handler).
    static std::string GetString(const std::string& key);

#if LOCALIZATION_STORE_DESCRIPTIONS
    // Translator-facing context for a key, for editor/debug tooling only -
    // never shown in-game. Empty if the key isn't found.
    static std::string GetDescription(const std::string& key);
#endif

    // Scans text for ${key} tokens and replaces each with its localized
    // value; everything else is copied through unchanged. A '$' not
    // followed by a well-formed {key} is left as a literal character, so
    // strings that merely contain a dollar sign (prices, etc.) are safe to
    // pass in. Cheap to call on text with no tokens at all - not even the
    // lock is touched in that case - so this is the function to call every
    // time you draw text.
    static std::string LocalizeString(std::string_view text);

    // ------------------------------------------------------------------
    // Per-frame caching helper
    // ------------------------------------------------------------------
    //
    // LocalizeString() re-scans and re-resolves its input on every call.
    // For text drawn every frame that rarely changes - menu labels, HUD
    // prompts - wrap the source text in a CachedText once and call Get()
    // each frame instead: it only re-resolves when the language changes or
    // is hot-reloaded, so a steady-state Get() is a single atomic load.
    //
    //   Localisation::CachedText title("${PAUSE_TITLE}");
    //   ...
    //   DrawText(title.Get()); // called every frame; cheap
    //
    // Not internally synchronized - see the Thread safety note at the top
    // of this file. Give each thread/widget its own instance.
    class CachedText
    {
    public:
        CachedText() = default;
        explicit CachedText(std::string sourceText);

        // Changes the template text (e.g. "${PAUSE_TITLE}") and forces the
        // next Get() to re-resolve it.
        void SetSourceText(std::string sourceText);

        const std::string& GetSourceText() const { return m_source; }

        // Returns the resolved text, re-resolving first if the language has
        // changed or been reloaded since the last call.
        const std::string& Get();

    private:
        static constexpr uint32_t kInvalidVersion = 0xFFFFFFFFu;

        std::string m_source;
        std::string m_resolved;
        uint32_t m_cachedVersion = kInvalidVersion;
    };

    // ------------------------------------------------------------------
    // Diagnostics
    // ------------------------------------------------------------------

    // Called with a human-readable message whenever a load fails or a
    // lookup falls through to the missing-key placeholder (at most once per
    // distinct missing key, to keep this safe to leave enabled while a
    // menu with a missing key is on screen). Defaults to printing to
    // stderr; set your own to route into the engine's logger. Your handler
    // may be called from any thread and should be safe for that (or simply
    // fast and lock-protected on your end, e.g. pushing onto a queue).
    static void SetWarningHandler(std::function<void(const std::string&)> handler);

private:
    struct Entry
    {
        std::string value;
#if LOCALIZATION_STORE_DESCRIPTIONS
        std::string description;
#endif
    };

    // Hashes std::string / std::string_view / const char* identically, so
    // an unordered_map<std::string, ...> keyed by this hasher can be
    // queried with a std::string_view (e.g. a slice of a larger string)
    // without allocating a temporary std::string just to do the lookup.
    struct TransparentStringHash
    {
        using is_transparent = void;
        size_t operator()(std::string_view sv) const noexcept { return std::hash<std::string_view>{}(sv); }
        size_t operator()(const std::string& s) const noexcept { return std::hash<std::string_view>{}(s); }
        size_t operator()(const char* s) const noexcept { return std::hash<std::string_view>{}(s); }
    };

    using EntryMap = std::unordered_map<std::string, Entry, TransparentStringHash, std::equal_to<>>;

    struct LanguageData
    {
        EntryMap entries;
    };

    // Pure w.r.t. static state: reads only the folder/code it's given and
    // writes only into outData, so it's safe to call from any thread
    // without holding s_dataMutex (that's the point - it lets writer calls
    // do their disk I/O without blocking readers).
    static bool LoadLanguageFromDisk(const std::string& rootFolder, const std::string& languageCode,
                                      LanguageData& outData);
    static bool ParseLanguageFile(const std::string& filePath, LanguageData& outData);

    // Assumes the caller already holds at least a shared lock on s_dataMutex.
    static const std::string* FindValue(std::string_view key);
    // Assumes the caller already holds at least a shared lock on s_dataMutex
    // (reads s_currentLanguageCode directly).
    static void ReportMissingKey(std::string_view key);

    static void ReportWarning(const std::string& message);
    static void ClearMissingKeyCache();

    // Guards every member below except s_version (which is atomic and
    // deliberately outside this lock - see GetVersion) and the diagnostics
    // members (which have their own, separate mutex so recording a missing
    // key never has to wait behind a writer, and vice versa).
    static std::shared_mutex s_dataMutex;

    static std::string s_rootFolder;
    static std::string s_defaultLanguageCode;
    static std::string s_currentLanguageCode;

    // Every language ever loaded this run, keyed by language code, so
    // switching back to a previously-active language is instant. Element
    // addresses in an unordered_map are stable across insertion, so the
    // two pointers below stay valid as languages are added.
    static std::unordered_map<std::string, LanguageData> s_languages;
    static LanguageData* s_currentLanguageData;
    static LanguageData* s_defaultLanguageData;

    static std::atomic<uint32_t> s_version;

    static std::mutex s_diagnosticsMutex; // guards s_reportedMissingKeys and s_warningHandler
    static std::unordered_set<std::string, TransparentStringHash, std::equal_to<>> s_reportedMissingKeys;
    static std::function<void(const std::string&)> s_warningHandler;
};
