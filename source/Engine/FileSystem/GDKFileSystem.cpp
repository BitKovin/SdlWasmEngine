#include "GDKFileSystem.h"

#include <Logger.hpp>

// ============================================================================
//  GDK / XGameSave implementation
// ============================================================================
#if defined(GDK_GAMESAVE_SUPPORTED)

#include <XAsync.h>
#include <set>
#include <string>

static HRESULT BlockUntilComplete(XAsyncBlock& async)
{
    return XAsyncGetStatus(&async, /*wait=*/true);
}

// ============================================================================
//  Construction / Init / Shutdown
// ============================================================================

GDKFileSystem::GDKFileSystem(const char* scid)
    : m_scid(scid ? scid : "")
{}

GDKFileSystem::~GDKFileSystem()
{
    Shutdown();
}

bool GDKFileSystem::Init()
{
    HRESULT hr = XTaskQueueCreate(
        XTaskQueueDispatchMode::ThreadPool,
        XTaskQueueDispatchMode::ThreadPool,
        &m_queue);

    if (FAILED(hr))
    {
        Logger::Error("[GDKFileSystem] XTaskQueueCreate failed: %08X\n", hr);
        return false;
    }

    // Use an event so we don't block the message pump
    HANDLE userEvent = CreateEventEx(nullptr, nullptr, 0, EVENT_ALL_ACCESS);

    XAsyncBlock* userAsync = new XAsyncBlock{};
    userAsync->queue = m_queue;
    userAsync->context = userEvent;
    userAsync->callback = [](XAsyncBlock* async)
        {
            SetEvent(reinterpret_cast<HANDLE>(async->context));
        };

    hr = XUserAddAsync(XUserAddOptions::AddDefaultUserAllowingUI, userAsync);
    if (FAILED(hr))
    {
        Logger::Error("[GDKFileSystem] XUserAddAsync start failed: %08X\n", hr);
        CloseHandle(userEvent);
        delete userAsync;
        return false;
    }

    // Pump messages while waiting so the sign-in UI can complete
    MSG msg{};
    while (WaitForSingleObjectEx(userEvent, 0, TRUE) == WAIT_TIMEOUT)
    {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(10);
    }

    hr = XUserAddResult(userAsync, &m_user);
    CloseHandle(userEvent);
    delete userAsync;

    if (FAILED(hr))
    {
        Logger::Error("[GDKFileSystem] XUserAddResult failed: %08X\n", hr);
        return false;
    }

    // Provider init has no UI — blocking is fine here
    {
        XAsyncBlock async{};
        async.queue = m_queue;

        hr = XGameSaveInitializeProviderAsync(m_user, m_scid.c_str(), false, &async);
        if (FAILED(hr))
        {
            Logger::Error("[GDKFileSystem] XGameSaveInitializeProviderAsync start failed: %08X\n", hr);
            return false;
        }

        hr = BlockUntilComplete(async);
        if (FAILED(hr))
        {
            Logger::Error("[GDKFileSystem] XGameSaveInitializeProviderAsync async failed: %08X\n", hr);
            return false;
        }

        hr = XGameSaveInitializeProviderResult(&async, &m_provider);
        if (FAILED(hr))
        {
            Logger::Error("[GDKFileSystem] XGameSaveInitializeProviderResult failed: %08X\n", hr);
            return false;
        }
    }

    Logger::Info("[GDKFileSystem] Initialized successfully with SCID '%s'\n", m_scid.c_str());
    return true;
}

void GDKFileSystem::Shutdown()
{
    if (m_provider) { XGameSaveCloseProvider(m_provider);  m_provider = nullptr; }
    if (m_user) { XUserCloseHandle(m_user);             m_user = nullptr; }
    if (m_queue)
    {
        XTaskQueueTerminate(m_queue, /*wait=*/true, nullptr, nullptr);
        XTaskQueueCloseHandle(m_queue);
        m_queue = nullptr;
    }
}

// ============================================================================
//  Path encoding / decoding
//
//  '/' in a relative path is replaced by "~~" in the blob name.
//  "~~" was chosen because it is highly unlikely to appear in real file names
//  and is valid in GDK blob names.
//
//  "SaveData/Profiles/profile1/saves/quicksave.sav"
//      → strip prefix → "Profiles/profile1/saves/quicksave.sav"
//      → encode       → "Profiles~~profile1~~saves~~quicksave.sav"   (blob name)
//
//  Decoding is the exact reverse, so GetFilesInPath can reconstruct the path
//  from any blob name without any additional metadata.
// ============================================================================

std::string GDKFileSystem::PathToBlob(const std::string& relPath)
{
    std::string result;
    result.reserve(relPath.size());

    for (char c : relPath)
        c == '/' ? result += kDirSeparator : result += c;

    return result;
}

std::string GDKFileSystem::BlobToPath(const std::string& blobName)
{
    std::string result;
    result.reserve(blobName.size());

    const std::string sep(kDirSeparator);
    const size_t sepLen = sep.size();

    for (size_t i = 0; i < blobName.size(); )
    {
        if (blobName.compare(i, sepLen, sep) == 0)
        {
            result += '/';
            i += sepLen;
        }
        else
        {
            result += blobName[i++];
        }
    }

    return result;
}

std::string GDKFileSystem::StripSavePrefix(const std::string& path)
{
    if (path.rfind(kSaveDataPrefix, 0) == 0)
        return path.substr(std::strlen(kSaveDataPrefix));
    return path;
}

// ============================================================================
//  Low-level blob I/O  (all inside kSaveContainer)
// ============================================================================

bool GDKFileSystem::WriteBlobInternal(const std::string& blobName,
    const void* data,
    uint32_t           dataSize)
{
    if (!m_provider) return false;

	Logger::Info("[GDKFileSystem] Writing blob '%s' (%u bytes)\n", blobName.c_str(), dataSize);

    XGameSaveContainerHandle container = nullptr;
    HRESULT hr = XGameSaveCreateContainer(m_provider, kSaveContainer, &container);
    if (FAILED(hr)) return false;

    bool success = false;

    XGameSaveUpdateHandle update = nullptr;
    hr = XGameSaveCreateUpdate(container, blobName.c_str(), &update);
    if (SUCCEEDED(hr))
    {
        hr = XGameSaveSubmitBlobWrite(update, blobName.c_str(),
            static_cast<const uint8_t*>(data), dataSize);
        if (SUCCEEDED(hr))
        {
            XAsyncBlock async{};
            async.queue = m_queue;

            hr = XGameSaveSubmitUpdateAsync(update, &async);
            if (SUCCEEDED(hr) && SUCCEEDED(BlockUntilComplete(async)))
                hr = XGameSaveSubmitUpdateResult(&async);

            success = SUCCEEDED(hr);
        }
        XGameSaveCloseUpdate(update);
    }

    XGameSaveCloseContainer(container);
    return success;
}

std::optional<std::vector<uint8_t>> GDKFileSystem::ReadBlobInternal(const std::string& blobName)
{
    if (!m_provider) return std::nullopt;

    XGameSaveContainerHandle container = nullptr;
    HRESULT hr = XGameSaveCreateContainer(m_provider, kSaveContainer, &container);
    if (FAILED(hr)) return std::nullopt;

    std::optional<std::vector<uint8_t>> result = std::nullopt;

    do
    {
        const char* names[] = { blobName.c_str() };
        XAsyncBlock readAsync{};
        readAsync.queue = m_queue;

        hr = XGameSaveReadBlobDataAsync(container, names, 1, &readAsync);
        if (FAILED(hr) || FAILED(BlockUntilComplete(readAsync)))
            break;

        size_t resultBufferSize = 0;
        hr = XAsyncGetResultSize(&readAsync, &resultBufferSize);
        if (FAILED(hr) || resultBufferSize == 0)
            break;

        std::vector<uint8_t> resultBuffer(resultBufferSize);
        uint32_t resultCount = 0;

        hr = XGameSaveReadBlobDataResult(
            &readAsync,
            resultBufferSize,
            reinterpret_cast<XGameSaveBlob*>(resultBuffer.data()),
            &resultCount);

        if (FAILED(hr) || resultCount == 0)
            break;

        const auto* blob = reinterpret_cast<const XGameSaveBlob*>(resultBuffer.data());
        if (blob->data == nullptr || blob->info.size == 0)
            break;

        result = std::vector<uint8_t>(blob->data, blob->data + blob->info.size);

    } while (false);

    XGameSaveCloseContainer(container);
    return result;
}

// ============================================================================
//  IFileSystem save-data overrides
// ============================================================================

bool GDKFileSystem::WriteSaveFile(const std::string& path, const std::string& content)
{
    const std::string blob = PathToBlob(StripSavePrefix(path));
    return WriteBlobInternal(blob, content.data(), static_cast<uint32_t>(content.size()));
}

std::optional<std::string> GDKFileSystem::ReadSaveFile(const std::string& path)
{
    const std::string blob = PathToBlob(StripSavePrefix(path));
    auto bytes = ReadBlobInternal(blob);
    if (!bytes) return std::nullopt;
    return std::string(reinterpret_cast<const char*>(bytes->data()), bytes->size());
}

bool GDKFileSystem::WriteSaveFileBinary(const std::string& path, const std::vector<uint8_t>& data)
{
    const std::string blob = PathToBlob(StripSavePrefix(path));
    return WriteBlobInternal(blob, data.data(), static_cast<uint32_t>(data.size()));
}

std::optional<std::vector<uint8_t>> GDKFileSystem::ReadSaveFileBinary(const std::string& path)
{
    const std::string blob = PathToBlob(StripSavePrefix(path));
    return ReadBlobInternal(blob);
}

// ============================================================================
//  GetFilesInPath
//
//  Opens kSaveContainer and enumerates every blob via XGameSaveEnumerateBlobInfoByName.
//  Each blob name is decoded back to a path with BlobToPath, then filtered to
//  find immediate children of the requested directory.
//
//  Because the full path lives in the blob name, this works correctly even
//  when files are added or deleted externally — no manifest can go stale.
//
//  Example — container holds:
//    "Preferences~~videoSettings.json"
//    "Preferences~~audioSettings.json"
//    "Profiles~~profile1~~saves~~quicksave.sav"
//    "Profiles~~profile1~~saves~~autosave.sav"
//
//  GetFilesInPath("SaveData/Profiles/profile1/saves/")
//    prefix  = "Profiles/profile1/saves/"
//    matches "Profiles/profile1/saves/quicksave.sav" → component "quicksave.sav"
//    matches "Profiles/profile1/saves/autosave.sav"  → component "autosave.sav"
//    result  = ["quicksave.sav", "autosave.sav"]
//
//  GetFilesInPath("SaveData/")
//    prefix  = ""
//    matches all four blobs → first components "Preferences", "Profiles"
//    result  = ["Preferences", "Profiles"]
// ============================================================================

std::vector<std::string> GDKFileSystem::GetFilesInPath(const std::string& path)
{
    if (path.rfind(kSaveDataPrefix, 0) != 0)
        return NativeFileSystem::GetFilesInPath(path);

    if (!m_provider) return {};

    XGameSaveContainerHandle container = nullptr;
    HRESULT hr = XGameSaveCreateContainer(m_provider, kSaveContainer, &container);
    if (FAILED(hr)) return {};

    std::string prefix = StripSavePrefix(path);
    if (!prefix.empty() && prefix.back() != '/')
        prefix += '/';

    struct EnumCtx
    {
        const std::string* prefix;
        std::set<std::string>    seen;
        std::vector<std::string> results;
    } ctx;
    ctx.prefix = &prefix;

    // Correct signature: (container, prefix, context, callback)
    XGameSaveEnumerateBlobInfoByName(
        container,
        /*blobNamePrefix=*/nullptr,
        &ctx,
        [](const XGameSaveBlobInfo* info, void* raw) -> bool
        {
            auto* c = static_cast<EnumCtx*>(raw);
            const std::string decoded = GDKFileSystem::BlobToPath(info->name);

            if (decoded.rfind(*c->prefix, 0) != 0)
                return true;

            const std::string remainder = decoded.substr(c->prefix->size());
            if (remainder.empty())
                return true;

            const size_t slash = remainder.find('/');
            const std::string component = (slash == std::string::npos)
                ? remainder
                : remainder.substr(0, slash);

            if (c->seen.insert(component).second)
                c->results.push_back(component);

            return true;
        });

    XGameSaveCloseContainer(container);
    return ctx.results;
}

#endif