#pragma once

#if __EMSCRIPTEN__
// Disable EFX on Web builds
#define DISABLE_EFX
#endif



#ifdef DISABLE_EFX
#  define AL_ALEXT_PROTOTYPES
#  include <AL/al.h>
#  include <AL/alc.h>
#else
#  define AL_ALEXT_PROTOTYPES
#  include <AL/al.h>
#  include <AL/alc.h>
#  include <AL/alext.h>
#endif

#include <cmath>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <algorithm>
#include "SoundInstanceBase.h"
#include "../glm.h"
#include "../Time.hpp"

#include "SoundBufferData.h"

class SoundInstance : public SoundInstanceBase {
public:
    // ─── Construction & Destruction ────────────────────────────────────

    /// buffer: pre-loaded OpenAL buffer
    /// isStereo: true → non-spatial stereo; false → 3D spatial mono
    SoundInstance(SoundBufferData buffer)
      : _bufferData(buffer)
    {

        _isStereo = buffer.stereo;

        _duration = buffer.duration;

        // Ensure pool is initialized once
        SourcePool::Init();

        // Start in "not playing" state; we will Acquire in Play()
    }

    ~SoundInstance() {
        Stop();
    }

    /// Begins or resumes playback. Will acquire a source if needed.
    void Play();


    /// Stops and resets playback
    void Stop();

    /// Must be called every frame to advance virtual playhead and reclaim sources
    void Update(float deltaTime);

    // ─── Playback Control ───────────────────────────────────────────────

    float  ConeInnerAngle = 360.0f;
    float  ConeOuterAngle = 360.0f;
    float  ConeOuterGain  = 1.0f;


    bool   EnableFilter   = false;
    float  LowPassGain    = 1.0f;
    float  LowPassGainHF  = 1.0f;

    bool   EnableEcho     = false;
    float  EchoDelay      = 0.1f;
    float  EchoLRDelay    = 0.1f;
    float  EchoDamping    = 0.5f;
    float  EchoFeedback   = 0.5f;
    float  EchoSpread     = 0.5f;

    bool   EnableReverb   = false;
    float  ReverbDensity  = 0.5f;
    float  ReverbGain     = 0.5f;
    float  ReverbGainHF   = 0.3f;
    float  ReverbDecayTime= 1.0f;



protected:
    bool IsGamePaused() const;
    float GetPitchScale() const;
    float GetFinalVolume() const;

private:
    // ─── Internal State ────────────────────────────────────────────────

    SoundBufferData   _bufferData       = SoundBufferData();
    ALuint   _source       = 0;
    bool     _isStereo     = false;

    bool     _active       = false;
    float    _virtualOffset= 0.0f;
    float    _duration     = 0.0f;

#ifndef DISABLE_EFX
    ALuint   _filter       = 0;
    ALuint   _effectEcho   = 0, _slotEcho    = 0;
    ALuint   _effectReverb = 0, _slotReverb  = 0;
#endif

    // ─── Try to acquire an OpenAL source from the pool ───────────────────

    void TryAcquire() {
        if (_source != 0) return;
        if (!_active)    return;
        alGetError();
#ifdef __EMSCRIPTEN__
        // On Web builds, always gen a fresh source (no pooling)
        ALuint src = 0;
        alGenSources(1, &src);
        if (alGetError() == AL_NO_ERROR) {
            _source = src;
        }
#else
        // Desktop: pooled path
        _source = SourcePool::Acquire(_isStereo, this);
#endif

        if (_source) alSourcei(_source, AL_BUFFER, _bufferData.buffer);
    }

    // ─── Release back to pool ────────────────────────────────────────────

    void ReleaseSource() 
    {
        if (!_source) return;
#ifdef __EMSCRIPTEN__
        alDeleteSources(1, &_source);
#else
        SourcePool::Release(_source, _isStereo, this);
#endif
        _source = 0;
    }

    // ─── Parameter application ──────────────────────────────────────────

    void UpdateSourceParams();

    float ComputeDistanceGain() {
        ALfloat lp[3];
        alGetListenerfv(AL_POSITION, lp);
        glm::vec3 listener(lp[0], lp[1], lp[2]);
        float d = glm::length(Position - listener);
        if (d < MinDistance) d = MinDistance;
        if (d >= MaxDistance) return 0.0f;
        float t = (d - MinDistance) / (MaxDistance - MinDistance);
        return GetFinalVolume() * std::pow(1.0f - t, 2.0f);
    }

#ifndef DISABLE_EFX
    void EnsureEFX() {
        ALCcontext* ctx = alcGetCurrentContext();
        if (!ctx) return;
        ALCdevice* dev = alcGetContextsDevice(ctx);
        if (!alcIsExtensionPresent(dev, "ALC_EXT_EFX")) return;

        if (EnableFilter && _filter == 0) {
            alGenFilters(1, &_filter);
            alFilteri(_filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
        }
        if (EnableEcho && _effectEcho == 0) {
            alGenEffects(1, &_effectEcho);
            alEffecti(_effectEcho, AL_EFFECT_TYPE, AL_EFFECT_ECHO);
            alEffectf(_effectEcho, AL_ECHO_DELAY,   EchoDelay);
            alEffectf(_effectEcho, AL_ECHO_LRDELAY, EchoLRDelay);
            alEffectf(_effectEcho, AL_ECHO_DAMPING,  EchoDamping);
            alEffectf(_effectEcho, AL_ECHO_FEEDBACK, EchoFeedback);
            alEffectf(_effectEcho, AL_ECHO_SPREAD,   EchoSpread);
            alGenAuxiliaryEffectSlots(1, &_slotEcho);
            alAuxiliaryEffectSloti(_slotEcho, AL_EFFECTSLOT_EFFECT, _effectEcho);
        }
        if (EnableReverb && _effectReverb == 0) {
            alGenEffects(1, &_effectReverb);
            alEffecti(_effectReverb, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
            alEffectf(_effectReverb, AL_REVERB_DENSITY,    ReverbDensity);
            alEffectf(_effectReverb, AL_REVERB_GAIN,       ReverbGain);
            alEffectf(_effectReverb, AL_REVERB_GAINHF,     ReverbGainHF);
            alEffectf(_effectReverb, AL_REVERB_DECAY_TIME, ReverbDecayTime);
            alGenAuxiliaryEffectSlots(1, &_slotReverb);
            alAuxiliaryEffectSloti(_slotReverb, AL_EFFECTSLOT_EFFECT, _effectReverb);
        }
    }

    void ApplyFilter() {
        alFilterf(_filter,   AL_LOWPASS_GAIN,   LowPassGain);
        alFilterf(_filter,   AL_LOWPASS_GAINHF, LowPassGainHF);
        alSourcei (_source,  AL_DIRECT_FILTER,  _filter);
    }
#endif

    // ─── Static Pool Manager ────────────────────────────────────────────

    struct SourcePool 
    {
        inline static std::vector<ALuint>  freeMono, freeStereo;
        inline static size_t               allocatedMono = 0, allocatedStereo = 0;
        inline static size_t               maxMono = 0, maxStereo = 0;
        inline static bool                 initialized = false;

        struct OwnerInfo {
            SoundInstance* inst;
            uint64_t       timestamp;
        };
        inline static std::unordered_map<ALuint, OwnerInfo> liveOwners;
        inline static uint64_t globalTimestamp = 0;

        static void Init();

        static ALuint Acquire(bool stereo, SoundInstance* requester);

        static void Release(ALuint src, bool stereo, SoundInstance* inst);
    };
};
