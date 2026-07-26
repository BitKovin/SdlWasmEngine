#pragma once

// Disable EFX on Emscripten / Web builds.
#if defined(__EMSCRIPTEN__)
#  define DISABLE_EFX
#endif

#define AL_ALEXT_PROTOTYPES
#include <AL/al.h>
#include <AL/alc.h>
#ifndef DISABLE_EFX
#  include <AL/alext.h>
#endif

#include <cmath>
#include <cstdint>
#include <limits>
#include <memory>
#include <unordered_map>
#include <vector>

#include "SoundInstanceBase.h"
#include "SoundBufferData.h"
#include "../glm.h"
#include "../Time.hpp"

class SoundInstance : public SoundInstanceBase
{
public:
    explicit SoundInstance(SoundBufferData buffer);
    ~SoundInstance();

    // All public methods are thread-safe — guarded by SoundManager::audioMutex.
    void Play()   override;
    void Stop()   override;
    void Update(float deltaTime) override;
    bool IsPlaying() override;

    // ── Per-source EFX / cone parameters ────────────────────────────────────
    float ConeInnerAngle  = 360.0f;
    float ConeOuterAngle  = 360.0f;
    float ConeOuterGain   = 1.0f;

    bool  EnableFilter    = false;
    float LowPassGain     = 1.0f;
    float LowPassGainHF   = 1.0f;

    bool  EnableEcho      = false;
    float EchoDelay       = 0.1f;
    float EchoLRDelay     = 0.1f;
    float EchoDamping     = 0.5f;
    float EchoFeedback    = 0.5f;
    float EchoSpread      = 0.5f;

    bool  EnableReverb    = false;
    float ReverbDensity   = 0.5f;
    float ReverbGain      = 0.5f;
    float ReverbGainHF    = 0.3f;
    float ReverbDecayTime = 1.0f;

    /// Called once from SoundManager::Initialize() AFTER both AL contexts exist.
    static void InitPool(ALCcontext* monoCtx, ALCcontext* stereoCtx);

    static void DestroyPool();

protected:
    bool  IsGamePaused()   const override;
    float GetPitchScale()  const override;
    float GetFinalVolume() const override;

private:
    /// Returns the OpenAL context that owns this instance's buffer and sources.
    ALCcontext* GetContext() const;

    SoundBufferData _bufferData;
    ALuint  _source        = 0;
    bool    _isStereo      = false;
    bool    _active        = false;
    float   _virtualOffset = 0.0f; ///< Seconds into clip; advances even without a live source.
    float   _duration      = 0.0f;

#ifndef DISABLE_EFX
    ALuint _filter       = 0;
    ALuint _effectEcho   = 0, _slotEcho   = 0;
    ALuint _effectReverb = 0, _slotReverb = 0;
#endif

    // ── Internal helpers ─────────────────────────────────────────────────────
    // All assume SoundManager::audioMutex is already held by the caller.

    void TryAcquire();
    void ReleaseSource();
    float GetDistanceFade(float distance) const;
    float CurrentDistanceToListener() const;
    void UpdateSourceParams();

#ifndef DISABLE_EFX
    void EnsureEFX();
    void ApplyFilter();
#endif

    // ── Static source pool ───────────────────────────────────────────────────
    /// Manages a shared pool of AL sources split across two contexts:
    ///   freeMono   → contextMono   (HRTF-enabled, for 3D mono sounds)
    ///   freeStereo → contextStereo (flat stereo,   for 2D/UI sounds)
    ///
    /// The "stereo" flag throughout the pool always maps to this context split —
    /// never mix sources across contexts.
    struct SourcePool
    {
        struct OwnerInfo
        {
            SoundInstance* inst      = nullptr;
            uint64_t       timestamp = 0;
        };

        // Set explicitly by InitPool() — no dependency on "current" context.
        inline static ALCcontext* monoContext   = nullptr;
        inline static ALCcontext* stereoContext = nullptr;

        inline static std::vector<ALuint>                    freeMono, freeStereo;
        inline static size_t                                 allocatedMono   = 0;
        inline static size_t                                 allocatedStereo = 0;
        inline static size_t                                 maxMono   = 128;
        inline static size_t                                 maxStereo = 32;
        inline static bool                                   initialized = false;

        inline static std::unordered_map<ALuint, OwnerInfo>  liveOwners;
        inline static uint64_t                               globalTimestamp = 0;

        static void   Init(ALCcontext* mono, ALCcontext* stereo);
        static ALuint Acquire(bool stereo, SoundInstance* requester);
        static void   Release(ALuint src, bool stereo);

        static void Destroy();

    private:
        /// Stop src, detach its buffer, and clear all EFX sends.
        /// Call before returning a source to the free list or repurposing it.
        static void ResetSourceState(ALuint src);
    };
};
