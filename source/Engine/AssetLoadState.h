#pragma once
#include <atomic>
#include <cstdint>

// Cross-thread readiness signal for a lazily-streamed asset (Texture or
// roj::SkinnedModel). Every field is a plain atomic load/store - never a
// lock - so Game/Render can check "is this ready yet" without ever blocking
// on the Loader thread. See AssetRegistry.cpp for who writes which field and
// from which thread.

// VisualImmediately is a *request-only* value - see AssetRegistry.h. Once an
// asset actually reaches full readiness, currentTier/queuedUpTo always read
// Visual, never VisualImmediately - the extra value exists purely so a
// call site (e.g. UI code loading a texture from the render thread) can ask
// for "synchronous, direct-to-GPU, right now" without being confused with
// the more general Visual escape hatch used by things like level loads.
enum class AssetLoadTier : uint8_t { None, Logic, Visual, VisualImmediately };

struct AssetLoadState
{
    std::atomic<AssetLoadTier> currentTier{ AssetLoadTier::None };

    // Bumped on every tier transition (load, partial unload, full unload).
    // Lets a mesh notice "something changed" with one atomic read instead of
    // comparing every field it might care about - see StaticMesh::PreFinalize().
    std::atomic<uint64_t> generation{ 0 };

    // Touched on every cache lookup, hit or miss, sync or async. Not used for
    // eviction yet - just recorded so a future streaming pass has the data it
    // needs without a separate tracking sweep.
    std::atomic<uint64_t> lastUsedFrame{ 0 };

    // Highest tier a load job has been queued for, whether or not it's
    // actually finished yet. Guards against double-queuing when the same
    // lazy load is requested again (e.g. two StaticMesh instances sharing a
    // path) before the Loader thread has picked up the first request.
    // UnloadToTier() resets this back down alongside currentTier.
    std::atomic<AssetLoadTier> queuedUpTo{ AssetLoadTier::None };
};
