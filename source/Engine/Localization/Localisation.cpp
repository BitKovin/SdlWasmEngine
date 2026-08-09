// Localisation.cpp
#include "Localisation.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <FileSystem/FileSystem.h>

#include <json.hpp>

// ------------------------------------------------------------------------
// Static storage
// ------------------------------------------------------------------------

std::shared_mutex Localisation::s_dataMutex;

std::string Localisation::s_rootFolder;
std::string Localisation::s_defaultLanguageCode;
std::string Localisation::s_currentLanguageCode;

std::unordered_map<std::string, Localisation::LanguageData> Localisation::s_languages;
Localisation::LanguageData* Localisation::s_currentLanguageData = nullptr;
Localisation::LanguageData* Localisation::s_defaultLanguageData = nullptr;

std::atomic<uint32_t> Localisation::s_version{0};

std::mutex Localisation::s_diagnosticsMutex;
std::unordered_set<std::string, Localisation::TransparentStringHash, std::equal_to<>>
    Localisation::s_reportedMissingKeys;

std::function<void(const std::string&)> Localisation::s_warningHandler =
    [](const std::string& message) { std::cerr << message << '\n'; };

// ------------------------------------------------------------------------
// Engine file IO - replace these two bodies with your engine's own file
// reading / directory listing. Keep the signatures the same. Safe to call
// from any thread (no shared/global state touched).
// ------------------------------------------------------------------------

bool Localisation::IO::ReadTextFile(const std::string& filePath, std::string& outContents)
{
    
    auto text = FileSystemEngine::ReadFile(filePath);
	outContents = text;
    
	return !text.empty();

}

std::vector<std::string> Localisation::IO::GetFilesInFolder(
    const std::string& folderPath,
    const std::string& extension)
{
    std::vector<std::string> result = FileSystemEngine::GetFilesInPath(folderPath);

    for (auto& file : result)
    {
		file = folderPath + "/" + file;
    }

    return result;
}

// ------------------------------------------------------------------------
// Setup
// ------------------------------------------------------------------------

bool Localisation::Initialize(const std::string& rootFolder, const std::string& defaultLanguageCode)
{
    // Do the (potentially slow) disk I/O before taking any lock, so this
    // never blocks a reader that's mid-LocalizeString on another thread.
    LanguageData data;
    const bool loaded = LoadLanguageFromDisk(rootFolder, defaultLanguageCode, data);

    std::unique_lock<std::shared_mutex> lock(s_dataMutex);

    s_rootFolder = rootFolder;
    s_defaultLanguageCode = defaultLanguageCode;
    s_currentLanguageCode = defaultLanguageCode;
    s_languages.clear();
    s_currentLanguageData = nullptr;
    s_defaultLanguageData = nullptr;

    if (!loaded)
    {
        ReportWarning("Localisation: failed to load default language '" + defaultLanguageCode +
                      "' from '" + rootFolder + "'");
        ClearMissingKeyCache();
        s_version.fetch_add(1, std::memory_order_relaxed);
        return false;
    }

    LanguageData& stored = s_languages[defaultLanguageCode];
    stored = std::move(data);
    s_defaultLanguageData = &stored;
    s_currentLanguageData = &stored;

    ClearMissingKeyCache();
    s_version.fetch_add(1, std::memory_order_relaxed);
    return true;
}

bool Localisation::SetLanguage(const std::string& languageCode)
{
    std::string rootFolder;
    bool needsLoad = true;

    {
        std::shared_lock<std::shared_mutex> lock(s_dataMutex);
        if (languageCode == s_currentLanguageCode && s_currentLanguageData)
            return true; // already active, nothing to do

        rootFolder = s_rootFolder;
        needsLoad = (s_languages.find(languageCode) == s_languages.end());
    }

    // Load off to the side, unlocked, if we haven't seen this language yet.
    LanguageData data;
    if (needsLoad)
    {
        if (!LoadLanguageFromDisk(rootFolder, languageCode, data))
        {
            ReportWarning("Localisation: failed to load language '" + languageCode + "', staying on '" +
                          GetCurrentLanguage() + "'");
            return false;
        }
    }

    std::unique_lock<std::shared_mutex> lock(s_dataMutex);

    auto it = s_languages.find(languageCode);
    if (it == s_languages.end())
        it = s_languages.emplace(languageCode, std::move(data)).first;
    else if (needsLoad)
        it->second = std::move(data); // rare race: another thread loaded it meanwhile too; ours wins

    s_currentLanguageCode = languageCode;
    s_currentLanguageData = &it->second;
    ClearMissingKeyCache();
    s_version.fetch_add(1, std::memory_order_relaxed);
    return true;
}

bool Localisation::ReloadCurrentLanguage()
{
    std::string rootFolder, currentCode, defaultCode;
    {
        std::shared_lock<std::shared_mutex> lock(s_dataMutex);
        if (s_currentLanguageCode.empty())
            return false; // Initialize() was never called
        rootFolder = s_rootFolder;
        currentCode = s_currentLanguageCode;
        defaultCode = s_defaultLanguageCode;
    }

    // Read both files unlocked; only the brief in-memory swap needs the lock.
    LanguageData freshCurrent;
    const bool okCurrent = LoadLanguageFromDisk(rootFolder, currentCode, freshCurrent);

    const bool distinctDefault = (defaultCode != currentCode);
    LanguageData freshDefault;
    const bool okDefault = distinctDefault ? LoadLanguageFromDisk(rootFolder, defaultCode, freshDefault) : true;

    std::unique_lock<std::shared_mutex> lock(s_dataMutex);

    bool ok = true;
    if (okCurrent)
    {
        if (auto it = s_languages.find(currentCode); it != s_languages.end())
            it->second = std::move(freshCurrent);
    }
    else
    {
        ReportWarning("Localisation: failed to reload language '" + currentCode + "'");
        ok = false;
    }

    if (distinctDefault)
    {
        if (okDefault)
        {
            if (auto it = s_languages.find(defaultCode); it != s_languages.end())
                it->second = std::move(freshDefault);
        }
        else
        {
            ReportWarning("Localisation: failed to reload default language '" + defaultCode + "'");
            ok = false;
        }
    }

    ClearMissingKeyCache();
    s_version.fetch_add(1, std::memory_order_relaxed);
    return ok;
}

std::string Localisation::GetCurrentLanguage()
{
    std::shared_lock<std::shared_mutex> lock(s_dataMutex);
    return s_currentLanguageCode;
}

uint32_t Localisation::GetVersion()
{
    // Deliberately outside s_dataMutex: this is just a "did anything
    // change" signal for CachedText, not a fence for the underlying data,
    // so a lock-free atomic read keeps the steady-state cost of every
    // CachedText::Get() call to a single atomic load.
    return s_version.load(std::memory_order_relaxed);
}

// ------------------------------------------------------------------------
// Loading (no static state touched - safe to call without s_dataMutex)
// ------------------------------------------------------------------------

bool Localisation::LoadLanguageFromDisk(const std::string& rootFolder, const std::string& languageCode,
                                         LanguageData& outData)
{
    outData.entries.clear();

    const std::string folder = rootFolder + "/" + languageCode;
    const std::vector<std::string> files = IO::GetFilesInFolder(folder, ".json");

    if (files.empty())
    {
        ReportWarning("Localisation: no .json files found in '" + folder + "'");
        return false;
    }

    bool anyLoaded = false;
    for (const std::string& file : files)
    {
        if (ParseLanguageFile(file, outData))
            anyLoaded = true;
    }

    return anyLoaded;
}

bool Localisation::ParseLanguageFile(const std::string& filePath, LanguageData& outData)
{
    std::string contents;
    if (!IO::ReadTextFile(filePath, contents))
    {
        ReportWarning("Localisation: could not read '" + filePath + "'");
        return false;
    }

    nlohmann::json json;
    try
    {
        json = nlohmann::json::parse(contents);
    }
    catch (const nlohmann::json::parse_error& e)
    {
        ReportWarning("Localisation: JSON error in '" + filePath + "': " + e.what());
        return false;
    }

    const auto entriesIt = json.find("entries");
    if (entriesIt == json.end() || !entriesIt->is_array())
    {
        ReportWarning("Localisation: '" + filePath + "' has no top-level 'entries' array");
        return false;
    }

    for (const auto& entryJson : *entriesIt)
    {
        const auto keyIt = entryJson.find("key");
        const auto valueIt = entryJson.find("value");
        if (keyIt == entryJson.end() || !keyIt->is_string() || valueIt == entryJson.end() ||
            !valueIt->is_string())
        {
            ReportWarning("Localisation: malformed entry in '" + filePath + "' (needs string 'key' and 'value')");
            continue;
        }

        std::string key = keyIt->get<std::string>();

        Entry entry;
        entry.value = valueIt->get<std::string>();
#if LOCALIZATION_STORE_DESCRIPTIONS
        const auto descIt = entryJson.find("description");
        if (descIt != entryJson.end() && descIt->is_string())
            entry.description = descIt->get<std::string>();
#endif

        const auto existingIt = outData.entries.find(key);
        if (existingIt != outData.entries.end())
        {
            ReportWarning("Localisation: duplicate key '" + key + "' in '" + filePath + "' - overwriting");
            existingIt->second = std::move(entry);
        }
        else
        {
            outData.entries.emplace(std::move(key), std::move(entry));
        }
    }

    return true;
}

// ------------------------------------------------------------------------
// Lookup
// ------------------------------------------------------------------------

const std::string* Localisation::FindValue(std::string_view key)
{
    // Caller must already hold at least a shared lock on s_dataMutex.
    if (s_currentLanguageData)
    {
        const auto it = s_currentLanguageData->entries.find(key);
        if (it != s_currentLanguageData->entries.end())
            return &it->second.value;
    }

    if (s_defaultLanguageData && s_defaultLanguageData != s_currentLanguageData)
    {
        const auto it = s_defaultLanguageData->entries.find(key);
        if (it != s_defaultLanguageData->entries.end())
            return &it->second.value;
    }

    return nullptr;
}

std::string Localisation::GetString(const std::string& key)
{
    std::shared_lock<std::shared_mutex> lock(s_dataMutex);

    if (const std::string* value = FindValue(key))
        return *value;

    ReportMissingKey(key);
    return "<" + key + ">";
}

#if LOCALIZATION_STORE_DESCRIPTIONS
std::string Localisation::GetDescription(const std::string& key)
{
    std::shared_lock<std::shared_mutex> lock(s_dataMutex);

    if (s_currentLanguageData)
    {
        const auto it = s_currentLanguageData->entries.find(key);
        if (it != s_currentLanguageData->entries.end())
            return it->second.description;
    }
    if (s_defaultLanguageData && s_defaultLanguageData != s_currentLanguageData)
    {
        const auto it = s_defaultLanguageData->entries.find(key);
        if (it != s_defaultLanguageData->entries.end())
            return it->second.description;
    }
    return {};
}
#endif

// ------------------------------------------------------------------------
// LocalizeString - the hot path. Called every time text is drawn.
//
// Fast-path: a string with no '$' at all costs one std::string_view::find
// and a single copy into the returned std::string - no lock, no token
// scanning, no map lookups.
//
// When '$' is present: the shared lock is taken once for the whole scan
// (not once per token), so concurrent LocalizeString calls from multiple
// threads only ever contend with an active writer, never with each other.
// ${key} tokens are looked up via a std::string_view slice directly into
// `text` (no substring allocation for the lookup itself, thanks to the
// transparent hasher/comparator on the entry maps), and everything else is
// copied through via bulk std::string::append calls.
// ------------------------------------------------------------------------

std::string Localisation::LocalizeString(std::string_view text)
{
    size_t dollarPos = text.find('$');
    if (dollarPos == std::string_view::npos)
        return std::string(text);

    std::string result;
    result.reserve(text.size() + 16);

    std::shared_lock<std::shared_mutex> lock(s_dataMutex);

    size_t pos = 0;
    while (dollarPos != std::string_view::npos)
    {
        result.append(text.data() + pos, dollarPos - pos);

        if (dollarPos + 1 < text.size() && text[dollarPos + 1] == '{')
        {
            const size_t keyStart = dollarPos + 2;
            const size_t closeBrace = text.find('}', keyStart);
            if (closeBrace != std::string_view::npos)
            {
                const std::string_view key = text.substr(keyStart, closeBrace - keyStart);
                if (const std::string* value = FindValue(key))
                {
                    result.append(*value);
                }
                else
                {
                    ReportMissingKey(key);
                    // Keep the original token visible rather than dropping it silently.
                    result.append(text.data() + dollarPos, closeBrace - dollarPos + 1);
                }

                pos = closeBrace + 1;
                dollarPos = text.find('$', pos);
                continue;
            }
        }

        // '$' wasn't the start of a well-formed ${...} token - keep it as a
        // literal character (e.g. "$100") and carry on scanning.
        result.push_back('$');
        pos = dollarPos + 1;
        dollarPos = text.find('$', pos);
    }

    result.append(text.data() + pos, text.size() - pos);
    return result;
}

// ------------------------------------------------------------------------
// CachedText - not internally synchronized, see the header's thread
// safety note. Each instance is expected to be owned by one thread.
// ------------------------------------------------------------------------

Localisation::CachedText::CachedText(std::string sourceText)
    : m_source(std::move(sourceText))
{
}

void Localisation::CachedText::SetSourceText(std::string sourceText)
{
    m_source = std::move(sourceText);
    m_cachedVersion = kInvalidVersion;
}

const std::string& Localisation::CachedText::Get()
{
    const uint32_t currentVersion = Localisation::GetVersion();
    if (m_cachedVersion != currentVersion)
    {
        m_resolved = Localisation::LocalizeString(m_source);
        m_cachedVersion = currentVersion;
    }
    return m_resolved;
}

// ------------------------------------------------------------------------
// Diagnostics
// ------------------------------------------------------------------------

void Localisation::SetWarningHandler(std::function<void(const std::string&)> handler)
{
    std::lock_guard<std::mutex> lock(s_diagnosticsMutex);
    s_warningHandler = std::move(handler);
}

void Localisation::ReportWarning(const std::string& message)
{
    std::function<void(const std::string&)> handlerCopy;
    {
        std::lock_guard<std::mutex> lock(s_diagnosticsMutex);
        handlerCopy = s_warningHandler;
    }
    if (handlerCopy)
        handlerCopy(message);
}

void Localisation::ReportMissingKey(std::string_view key)
{
    // Caller must already hold at least a shared lock on s_dataMutex (we
    // read s_currentLanguageCode directly below).
    bool shouldReport = false;
    {
        std::lock_guard<std::mutex> lock(s_diagnosticsMutex);
        if (s_reportedMissingKeys.find(key) == s_reportedMissingKeys.end())
        {
            s_reportedMissingKeys.emplace(key);
            shouldReport = true;
        }
    }

    if (shouldReport)
        ReportWarning("Localisation: missing key '" + std::string(key) + "' (language '" +
                      s_currentLanguageCode + "')");
}

void Localisation::ClearMissingKeyCache()
{
    std::lock_guard<std::mutex> lock(s_diagnosticsMutex);
    s_reportedMissingKeys.clear();
}
