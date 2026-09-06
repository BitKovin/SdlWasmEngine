#pragma once
#include <set>
#include "malloc_override.h"

#include "Shader.hpp"
#include "skinned_model.hpp"
#include "model.hpp"
#include "Texture.hpp"
#include "TextureCube.hpp"
#include "Logger.hpp"
#include "Video/Video.h"
#include "ThreadPool.h"
#include "AssetLoadState.h"
#include <vector>
#include <deque>
#include <mutex>
#include <atomic>
#include <functional>


class AssetRegistry
{

private:
    static std::unordered_map<std::string, Shader*> shaderCache;
    static std::unordered_map<std::string, Texture*> textureCache;
    static std::unordered_map<std::string, Video*> videoCache;
    static std::unordered_map<std::string, CubemapTexture*> textureCubeCache;
    static std::unordered_map<std::string, roj::SkinnedModel*> skinnedModelCache;
    static std::unordered_map<std::string, roj::SkinnedModel*> skinnedModelAnimationCache;

    static std::set<std::string> loadedAssetsDuringLoading;
    static std::set<std::string> constantlyLoaded;

    static inline bool loadingLevel = false;

    // Guards every cache map above plus the usage-tracking sets. Recursive
    // because e.g. ClearUnusedMemory() calls IsAssetUsed() while already
    // holding it. This only matters once loading can come from more than one
    // thread: gameplay code can call GetTextureFromFile/GetSkinnedModelFromFile
    // from the Game thread while ProcessPendingUploads() (Main thread)
    // registers an embedded GLB texture for a different model at the same time.
    static std::recursive_mutex cacheMutex;

    // Loader thread — single worker, same pattern as EngineMain's
    // GameUpdateSingleThreadPool: predictable thread count on platforms that
    // cap how many threads an app may create.
    static ThreadPool* loaderThreadPool;

    // Work handed from the Loader thread (or anywhere else that decoded
    // something but can't touch bgfx) to the main thread. See EnqueuePendingUpload/ProcessPendingUploads.
    struct PendingUpload
    {
        std::function<void()> Apply;
        size_t approxBytes = 0; // for ResourceStatistics - not used to gate the budget
    };
    static std::mutex uploadQueueMutex;
    static std::deque<PendingUpload> uploadQueue;

    static std::atomic<uint64_t> frameCounter;
    static void TouchUsage(AssetLoadState& state);

    // Parses just the Logic tier (skeleton/bones/animation clips - no
    // meshes, no bgfx) and commits it, synchronously, on whichever thread
    // calls it. Safe from any thread precisely because it never touches
    // bgfx. Caller holds cacheMutex.
    static void LoadLogicTierNow(roj::SkinnedModel* model, const std::string& path);

    // Background counterpart to LoadLogicTierNow() - same CPU-only,
    // bgfx-free work (SkipVisual=true), just queued onto the Loader thread
    // instead of run inline on the calling thread. Used during
    // constant-asset preloading (see GetSkinnedAnimationFromFile) so that
    // phase never blocks on even Logic-tier parsing.
    static void QueueLogicTierJob(roj::SkinnedModel* model, std::string path);
    static void QueueVisualTierJob(roj::SkinnedModel* model, std::string path);
    static void QueueTextureUploadJob(Texture* texture, std::string path);
    static void QueueTextureCubeUploadJob(CubemapTexture* texCube, std::string path);

    // Brings a model up to requestedTier without ever blocking the calling
    // thread on the expensive part:
    //   - Logic is loaded synchronously, right here (see LoadLogicTierNow) -
    //     that's what makes the default requestedTier == Logic return with
    //     usable bones/animations instead of an empty placeholder.
    //   - Visual (geometry + GPU upload) is still queued onto the Loader
    //     thread, exactly as before - that's the part that would cause a
    //     frame drop if done synchronously, so it never runs on this thread.
    // Caller holds cacheMutex.
    static void EnsureTierLazy(roj::SkinnedModel* model, const std::string& path, AssetLoadTier requestedTier);
    static void EnsureTextureTierLazy(Texture* texture, const std::string& path, AssetLoadTier requestedTier);
    static void EnsureTextureCubeTierLazy(CubemapTexture* texCube, const std::string& path, AssetLoadTier requestedTier);

    // Brings an already-registered object straight to Visual, synchronously,
    // right now, with a direct bgfx call - used for the loading-screen path
    // and the requestedTier == Visual/VisualImmediately escape hatches.
    // Caller holds cacheMutex.
    static void SynchronouslyFinishLoad(roj::SkinnedModel* model, const std::string& path);
    static void SynchronouslyFinishTextureLoad(Texture* texture, const std::string& path);
    static void SynchronouslyFinishTextureCubeLoad(CubemapTexture* texCube, const std::string& path);

public:

    static inline bool LoadingConstantAssets = false;

    // How many milliseconds ProcessPendingUploads() may spend per frame.
    // Time-based rather than byte-based - GPU upload cost varies too much by
    // hardware for a fixed byte budget to be portable.
    static inline float UploadBudgetMs = 0.5f;

    static void ClearMemory();
    static void ClearUnusedMemory();
    static bool IsAssetUsed(std::string filename);


    static void ReloadShaders();

    // requestedTier controls what happens outside a level load (loadingLevel
    // == false / LoadingConstantAssets == false):
    //   None  - register only, queue nothing. Fully caller-driven from here
    //           on (see RequestVisualLoad/UnloadToTier).
    //   Logic - default. For a model, its Logic tier (skeleton/bones/
    //           animation clips - no meshes, no bgfx) is loaded
    //           synchronously on the calling thread before returning, so
    //           the pointer you get back already has usable bones/
    //           animations. Its Visual tier (geometry + GPU upload) is
    //           queued onto the Loader thread and streams in over the next
    //           few frames. A texture has no meaningful Logic-only data, so
    //           this just queues its (only) decode+upload lazily, same as
    //           before. Either way, nothing here makes a bgfx call on the
    //           calling thread.
    //   Visual - escape hatch: forces the old fully-synchronous, blocking
    //           load right now, on the calling thread, regardless of context.
    //           This DOES call bgfx directly on the calling thread, so only
    //           ever pass it from a thread that's allowed to touch bgfx
    //           (main/render thread) - e.g. the level-load path below,
    //           which is why that path is unaffected by this distinction.
    //   VisualImmediately - identical to Visual (synchronous, direct bgfx
    //           upload, right now). Meant for UI code - e.g. loading a
    //           texture to show right now - that is guaranteed to be
    //           running on the render thread. Kept as its own value rather
    //           than reusing Visual purely so call sites self-document that
    //           thread guarantee instead of inheriting Visual's more general
    //           "regardless of context" contract.
    // Inside a level load (BeginLevelLoad/EndLevelLoad, or LoadingConstantAssets),
    // requestedTier is ignored - always synchronous, exactly as before, since
    // that's what the loading screen exists for. That path already assumes
    // it's running on a thread that may call bgfx (unchanged by this).
    static Texture* GetTextureFromFile(string filename, AssetLoadTier requestedTier = AssetLoadTier::Logic);

    // See GetTextureFromFile's comment for what requestedTier does. Default
    // is Visual (not Logic) here, unlike Texture/SkinnedModel - preserves
    // this call's previous always-synchronous behavior for any existing
    // call sites that don't pass a tier.
    static CubemapTexture* GetTextureCubeFromFile(string filename, AssetLoadTier requestedTier = AssetLoadTier::Visual);

    static void RegisterTexture(Texture* texture, string path);

    static Video* GetVideoFromFile(string filename);


    static std::string ReadFileToString(string filename);

    static void BeginLevelLoad();
    static void EndLevelLoad();

    static void MarkAsUsed(std::string filename);

    // See GetTextureFromFile's comment for what requestedTier does.
    static roj::SkinnedModel* GetSkinnedModelFromFile(const string& path, AssetLoadTier requestedTier = AssetLoadTier::Logic);
    static roj::SkinnedModel* GetSkinnedAnimationFromFile(const string& path, AssetLoadTier requestedTier = AssetLoadTier::Logic);

    // ── Tiered loading plumbing ──────────────────────────────────────────────
    // The functions above are lazy by default now (see requestedTier) - what
    // follows is how that laziness is driven and how a caller manages an
    // object it already has a pointer to.

    static void StartLoaderThread(); // call once at engine init
    static void StopLoaderThread();  // call once at engine shutdown
    static void AdvanceFrame();      // call once per frame - drives lastUsedFrame

    // Re-queues just the Visual tier for a model/texture that's currently
    // below it (e.g. after UnloadToTier dropped it, or after a GetXFromFile
    // call with requestedTier == None). No-op if already there or in flight.
    static void RequestVisualLoad(roj::SkinnedModel* model, string path);
    static void RequestVisualLoad(Texture* texture, string path);
    static void RequestVisualLoad(CubemapTexture* texCube, string path);

    // Drops everything above the requested tier. Never erases the cache
    // entry or deletes the object - same pointer, same cache entry, just
    // reads "not loaded" in the meantime. A real level change still goes
    // through ClearUnusedMemory()/BeginLevelLoad() as before. Safe to call
    // from any thread - it only enqueues; the actual bgfx::destroy calls run
    // on the main thread via ProcessPendingUploads, same as an upload.
    static void UnloadToTier(roj::SkinnedModel* model, AssetLoadTier tier);
    static void UnloadToTier(Texture* texture, AssetLoadTier tier); // tier is really just Visual vs None
    static void UnloadToTier(CubemapTexture* texCube, AssetLoadTier tier); // tier is really just Visual vs None

    // Call once per frame, from the very start of Render(), before
    // RenderLevel() - the only place allowed to touch bgfx for loading.
    static void ProcessPendingUploads(float msBudget);

    // Lets other systems that decode something off the main thread (e.g. an
    // embedded GLB texture found mid-parse) hand the bgfx-touching part off
    // to the same queue instead of calling bgfx directly from the Loader thread.
    static void EnqueuePendingUpload(std::function<void()> apply, size_t approxBytes = 0);

    // True if there's any asset-loading work outstanding, on either side of
    // the pipeline: still running/queued on the Loader thread, or finished
    // there and waiting for ProcessPendingUploads() to apply it on the main
    // thread. Useful for e.g. a loading screen deciding it's safe to advance.
    static bool HasPendingWork();

private:

    static void MarkModelTexturesAsUsed(roj::SkinnedModel* model, std::string path);

};