#include "SoundInstance.hpp"
#include "SoundManager.hpp"
#include "../EngineMain.h"
#include <Logger.hpp>

// =============================================================================
// SourcePool
// =============================================================================

void SoundInstance::InitPool(ALCcontext* monoCtx, ALCcontext* stereoCtx)
{
    SourcePool::Init(monoCtx, stereoCtx);
}

void SoundInstance::SourcePool::Init(ALCcontext* mono, ALCcontext* stereo)
{
    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
    if (initialized) return;
    initialized   = true;
    monoContext   = mono;
    stereoContext = stereo;

    // Query advertised source counts from the mono context's device.
    // We do this here so the pool always knows its real limits regardless of
    // which context is "current" at the time the first SoundInstance is built.
    alcMakeContextCurrent(mono);
    ALCdevice* dev = alcGetContextsDevice(mono);

    ALCint qMono = 0, qStereo = 0;
    if (dev) {
        alcGetIntegerv(dev, ALC_MONO_SOURCES,   1, &qMono);
        alcGetIntegerv(dev, ALC_STEREO_SOURCES, 1, &qStereo);
    }

    // Some drivers report 0 or astronomically large values — clamp to sane range.
    auto safeClamp = [](ALCint v, size_t lo, size_t hi) -> size_t {
        if (v <= 0 || static_cast<size_t>(v) > hi) return lo;
        return static_cast<size_t>(v);
    };
    maxMono   = safeClamp(qMono,   8, 512);
    maxStereo = safeClamp(qStereo, 4, 128);

    Logger::Info("[SourcePool] Initialized — maxMono=%zu, maxStereo=%zu", maxMono, maxStereo);
}

// Detach buffer and clear all EFX sends so the next owner starts from a clean state.
void SoundInstance::SourcePool::ResetSourceState(ALuint src)
{
    alSourceStop(src);
    alSourcei(src, AL_BUFFER, 0);
#ifndef DISABLE_EFX
    alSourcei (src, AL_DIRECT_FILTER,         AL_FILTER_NULL);
    alSource3i(src, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 0, AL_FILTER_NULL);
    alSource3i(src, AL_AUXILIARY_SEND_FILTER, AL_EFFECTSLOT_NULL, 1, AL_FILTER_NULL);
#endif
}

ALuint SoundInstance::SourcePool::Acquire(bool stereo, SoundInstance* requester)
{
    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);

    ALCcontext* ctx = stereo ? stereoContext : monoContext;
    alcMakeContextCurrent(ctx);
    alGetError();

    auto&        freePool  = stereo ? freeStereo  : freeMono;
    auto&        allocated = stereo ? allocatedStereo : allocatedMono;
    const size_t limit     = stereo ? maxStereo   : maxMono;
    const size_t inUse     = allocated - freePool.size();

    // ── 1. Reuse a previously released source ────────────────────────────────
    if (!freePool.empty()) {
        ALuint s = freePool.back();
        freePool.pop_back();
        liveOwners[s] = { requester, ++globalTimestamp };
        return s;
    }

    // ── 2. Allocate a new source if the device budget allows ─────────────────
    if (inUse < limit) {
        ALuint s = 0;
        alGenSources(1, &s);
        if (alGetError() == AL_NO_ERROR) {
            ++allocated;
            liveOwners[s] = { requester, ++globalTimestamp };
            return s;
        }
        Logger::Warning("[SourcePool] alGenSources failed despite inUse=%zu < limit=%zu",
                        inUse, limit);
    }

    // Stereo sources are never stolen: they are few and used for music / UI.
    if (stereo) return 0;

    // ── 3. Steal the lowest-priority, oldest, non-looping mono source ─────────
    //
    // Rules:
    //   • Only steal a source whose priority is strictly below the requester's —
    //     never interrupt an equally or more important sound.
    //   • Among candidates, prefer the one with the lowest priority.
    //   • Break ties by choosing the oldest acquisition (smallest timestamp).
    //   • Zero the victim's _source handle inside the lock so its next
    //     Update() tick sees _source==0 and enters the virtual-playhead path
    //     instead of calling AL on a source it no longer owns.

    ALuint   victimSrc  = 0;
    float    victimPrio = requester->Priority; // steal threshold
    uint64_t victimTime = std::numeric_limits<uint64_t>::max();

    // Cache once — IsGamePaused() reads the same engine flag for every instance.
    const bool gameIsPaused = requester->IsGamePaused();

    for (auto& [src, info] : liveOwners) {
        SoundInstance* inst = info.inst;
        if (inst->_isStereo) continue; // wrong context
        if (inst->Loop)      continue; // looping sounds must not be interrupted

        // During game pause, don't steal from a non-UI sound that is about to
        // be paused by its own Update() call. If we steal its source first,
        // _source becomes 0 before the pause branch runs, so alSourcePause()
        // is never called and the sound keeps playing.
        // Exception: a UI sound (game-pause exempt) may still steal freely.
        if (gameIsPaused && !inst->IsUISound && !requester->IsUISound) continue;

        const float    pr = inst->Priority;
        const uint64_t ts = info.timestamp;

        if (pr < victimPrio || (pr == victimPrio && ts < victimTime)) {
            victimPrio = pr;
            victimTime = ts;
            victimSrc  = src;
        }
    }

    if (victimSrc) {
        SoundInstance* victim = liveOwners[victimSrc].inst;

        // Null the victim's handle while we still hold the lock.
        victim->_source = 0;

        ResetSourceState(victimSrc);
        liveOwners[victimSrc] = { requester, ++globalTimestamp };

        Logger::Info("[SourcePool] Stole src=%u (prio=%.1f) for requester prio=%.1f",
                     victimSrc, victimPrio, requester->Priority);
        return victimSrc;
    }

    return 0; // All live sources have equal or higher priority — nothing to steal.
}

void SoundInstance::SourcePool::Release(ALuint src, bool stereo)
{
    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
    alcMakeContextCurrent(stereo ? stereoContext : monoContext);
    ResetSourceState(src);
    liveOwners.erase(src);
    (stereo ? freeStereo : freeMono).push_back(src);
}

// =============================================================================
// SoundInstance — construction / destruction
// =============================================================================

SoundInstance::SoundInstance(SoundBufferData buffer)
    : _bufferData(buffer)
    , _isStereo(buffer.stereo)
    , _duration(buffer.duration)
{
}

SoundInstance::~SoundInstance()
{
    Stop();
}

// =============================================================================
// SoundInstance — private helpers (caller holds SoundManager::audioMutex)
// =============================================================================

ALCcontext* SoundInstance::GetContext() const
{
    return _isStereo ? SourcePool::stereoContext : SourcePool::monoContext;
}

bool SoundInstance::IsGamePaused() const
{
    return EngineMain::MainInstance->Paused;
}

float SoundInstance::GetPitchScale() const
{
    return IsUISound ? 1.0f : Time::GetSoundFinalTimeScale();
}

float SoundInstance::GetFinalVolume() const
{
    return Volume * SoundManager::GlobalVolume * SoundManager::GetVolumeForSoundType(Type);
}

void SoundInstance::TryAcquire()
{
    if (_source || !_active) return;

    // Don't acquire a source for a non-UI sound while the game is paused.
    // The pause branch in Update() handles live sources via alSourcePause();
    // for virtual sounds (no source yet) staying sourceless is correct —
    // no audio is produced and the sound will re-acquire when the game unpauses.
    if (!IsUISound && IsGamePaused()) return;

    alGetError();

#ifdef __EMSCRIPTEN__
    alcMakeContextCurrent(GetContext());
    ALuint src = 0;
    alGenSources(1, &src);
    if (alGetError() == AL_NO_ERROR)
        _source = src;
#else
    _source = SourcePool::Acquire(_isStereo, this);
#endif

    if (!_source) return;

    alSourcei(_source, AL_BUFFER, _bufferData.buffer);

#ifndef DISABLE_EFX
    // Explicitly opt 3D mono sources into the HRTF spatializer.
    // Without this flag, some configurations fall back to simple equal-power
    // panning which hard-pans the signal to a single channel at ±90°.
    // AL_SOURCE_SPATIALIZE_SOFT is a no-op when HRTF is not active, so it is
    // always safe to set.
    if (!_isStereo && !Is2D && !IsUISound) {
        static const bool hasSpatialize =
            (alIsExtensionPresent("AL_SOFT_source_spatialize") == AL_TRUE);
        if (hasSpatialize)
            alSourcei(_source, AL_SOURCE_SPATIALIZE_SOFT, AL_TRUE);
    }
#endif
}

void SoundInstance::ReleaseSource()
{
    if (!_source) return;
#ifdef __EMSCRIPTEN__
    alcMakeContextCurrent(GetContext());
    alDeleteSources(1, &_source);
#else
    SourcePool::Release(_source, _isStereo);
#endif
    _source = 0;
}

static float SmoothStep(float t)
{
    t = std::clamp(t, 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}

float SoundInstance::GetDistanceFade(float distance) const
{
    if (_isStereo || Is2D || IsUISound)
        return 1.0f;

    if (distance <= MinDistance)
        return 1.0f;

    if (distance >= MaxDistance)
        return 0.0f;

    // Calculate linear fade (0.0 to 1.0)
    float fade = (distance - MinDistance) / (MaxDistance - MinDistance);

    // Invert the fade so it goes from 1.0 to 0.0
    float volume = 1.0f - fade;

    // Square the result for FMOD's non-linear concave curve
    return volume;
}

float SoundInstance::CurrentDistanceToListener() const
{
	if (_isStereo || Is2D || IsUISound)
		return 0.0f;
	const glm::vec3 listenerPos = Camera::finalizedPosition;
	return glm::distance(Position, listenerPos);
}

void SoundInstance::UpdateSourceParams()
{
    if (!_source) return;

    alcMakeContextCurrent(GetContext());

    alSourcei(_source, AL_LOOPING, Loop ? AL_TRUE : AL_FALSE);
    alSourcef(_source, AL_PITCH,   Pitch * GetPitchScale());

    if (_isStereo || Is2D || IsUISound) {
        // ── Non-spatial: head-relative, constant gain ────────────────────────
        alSourcei (_source, AL_SOURCE_RELATIVE, AL_TRUE);
        alSourcef (_source, AL_ROLLOFF_FACTOR,  0.0f);
        alSourcef (_source, AL_GAIN,            GetFinalVolume());
        alSource3f(_source, AL_POSITION,        0.0f, 0.0f, 0.0f);
        alSource3f(_source, AL_VELOCITY,        0.0f, 0.0f, 0.0f);
    }
    else {
        // ── 3D spatial ───────────────────────────────────────────────────────
        // Distance attenuation is handled by AL_LINEAR_DISTANCE_CLAMPED, set
        // once per context in SoundManager::Initialize(). That model reaches
        // exactly 0 at AL_MAX_DISTANCE, matching FMOD's default behaviour.
        // AL_GAIN is the base volume at AL_REFERENCE_DISTANCE (MinDistance);
        // OpenAL scales it down linearly from there.
        alSourcei (_source, AL_SOURCE_RELATIVE,    AL_FALSE);
        alSource3f(_source, AL_POSITION,           Position.x,  Position.y,  Position.z);
        alSource3f(_source, AL_VELOCITY,           Velocity.x,  Velocity.y,  Velocity.z);
        alSource3f(_source, AL_DIRECTION,          Direction.x, Direction.y, Direction.z);
        alSourcef (_source, AL_CONE_INNER_ANGLE,   ConeInnerAngle);
        alSourcef (_source, AL_CONE_OUTER_ANGLE,   ConeOuterAngle);
        alSourcef (_source, AL_CONE_OUTER_GAIN,    ConeOuterGain);

        const float distanceFade = GetDistanceFade(CurrentDistanceToListener());
        alSourcef(_source, AL_GAIN, GetFinalVolume() * distanceFade);

        alSourcef (_source, AL_REFERENCE_DISTANCE, MinDistance);
        alSourcef (_source, AL_MAX_DISTANCE,       MaxDistance);
        alSourcef (_source, AL_ROLLOFF_FACTOR,     1.0f);
    }

#ifndef DISABLE_EFX
    EnsureEFX();
    if (EnableFilter && _filter)
        ApplyFilter();
    if (EnableEcho   && _slotEcho)
        alSource3i(_source, AL_AUXILIARY_SEND_FILTER, _slotEcho,   0, AL_FILTER_NULL);
    if (EnableReverb && _slotReverb)
        alSource3i(_source, AL_AUXILIARY_SEND_FILTER, _slotReverb, 1, AL_FILTER_NULL);
#endif
}

#ifndef DISABLE_EFX
void SoundInstance::EnsureEFX()
{
    ALCdevice* dev = alcGetContextsDevice(alcGetCurrentContext());
    if (!dev || !alcIsExtensionPresent(dev, "ALC_EXT_EFX")) return;

    if (EnableFilter && !_filter) {
        alGenFilters(1, &_filter);
        alFilteri(_filter, AL_FILTER_TYPE, AL_FILTER_LOWPASS);
    }
    if (EnableEcho && !_effectEcho) {
        alGenEffects(1, &_effectEcho);
        alEffecti(_effectEcho, AL_EFFECT_TYPE,    AL_EFFECT_ECHO);
        alEffectf(_effectEcho, AL_ECHO_DELAY,     EchoDelay);
        alEffectf(_effectEcho, AL_ECHO_LRDELAY,   EchoLRDelay);
        alEffectf(_effectEcho, AL_ECHO_DAMPING,   EchoDamping);
        alEffectf(_effectEcho, AL_ECHO_FEEDBACK,  EchoFeedback);
        alEffectf(_effectEcho, AL_ECHO_SPREAD,    EchoSpread);
        alGenAuxiliaryEffectSlots(1, &_slotEcho);
        alAuxiliaryEffectSloti(_slotEcho, AL_EFFECTSLOT_EFFECT, _effectEcho);
    }
    if (EnableReverb && !_effectReverb) {
        alGenEffects(1, &_effectReverb);
        alEffecti(_effectReverb, AL_EFFECT_TYPE,        AL_EFFECT_REVERB);
        alEffectf(_effectReverb, AL_REVERB_DENSITY,     ReverbDensity);
        alEffectf(_effectReverb, AL_REVERB_GAIN,        ReverbGain);
        alEffectf(_effectReverb, AL_REVERB_GAINHF,      ReverbGainHF);
        alEffectf(_effectReverb, AL_REVERB_DECAY_TIME,  ReverbDecayTime);
        alGenAuxiliaryEffectSlots(1, &_slotReverb);
        alAuxiliaryEffectSloti(_slotReverb, AL_EFFECTSLOT_EFFECT, _effectReverb);
    }
}

void SoundInstance::ApplyFilter()
{
    alFilterf(_filter,  AL_LOWPASS_GAIN,   LowPassGain);
    alFilterf(_filter,  AL_LOWPASS_GAINHF, LowPassGainHF);
    alSourcei (_source, AL_DIRECT_FILTER,  _filter);
}
#endif

// =============================================================================
// SoundInstance — public API
// =============================================================================

bool SoundInstance::IsPlaying()
{
    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
    return _active;
}

void SoundInstance::Play()
{
    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);

    // No-op if already playing with a live source.
    if (_active && _source != 0) return;

    Paused         = false;
    _active        = true;
    _virtualOffset = 0.0f;

    TryAcquire();
    if (_source) {
        UpdateSourceParams();
        alSourcePlay(_source);
    }
}

void SoundInstance::Stop()
{
    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);

    if (_source) {
        alcMakeContextCurrent(GetContext());
        alSourceStop(_source);
        ReleaseSource();
    }

    _virtualOffset = 0.0f;
    _active        = false;
}

void SoundInstance::Update(float deltaTime)
{
    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);

    // Set context once at the top so every AL call below — including those in
    // the pause branch — goes to the right context without repeating this call.
    alcMakeContextCurrent(GetContext());

    const bool shouldPause = Paused || (!IsUISound && IsGamePaused());

    // ── Pause handling ───────────────────────────────────────────────────────
    // We use alSourcePause() rather than releasing the source back to the pool.
    // This means the OpenAL source stays paused at the hardware level even if
    // Update() is not called on subsequent frames (e.g. the game loop skips
    // audio updates while paused). The source is only released if it gets stolen
    // by a higher-priority sound.
    if (shouldPause) {
        if (_source) {
            ALint state = AL_PLAYING;
            alGetSourcei(_source, AL_SOURCE_STATE, &state);
            if (state == AL_PLAYING) {
                // Sync virtual offset from the hardware position before pausing
                // so that if the source is stolen during pause, re-acquisition
                // will seek to the correct location.
                ALfloat alOffset = 0.0f;
                alGetSourcef(_source, AL_SEC_OFFSET, &alOffset);
                _virtualOffset = alOffset;
                alSourcePause(_source);
            }
        }
        return;
    }

    if (!_active) return;

    _virtualOffset += deltaTime;

    if (GetDistanceFade(CurrentDistanceToListener()) <= 0.0001f) {
        if (_source)
            ReleaseSource();   // keep _active and _virtualOffset so it can come back
        return;
    }

    // ── Live source ───────────────────────────────────────────────────────────
    if (_source) {
        ALint state = AL_STOPPED;
        alGetSourcei(_source, AL_SOURCE_STATE, &state);

        if (state == AL_PAUSED) {
            // Game just unpaused — resume from the hardware's frozen position.
            UpdateSourceParams();
            alSourcePlay(_source);
            return;
        }

        if (state == AL_PLAYING) {
            UpdateSourceParams();
            return;
        }

        // AL_STOPPED: source finished naturally (or stolen and already zeroed).
        ReleaseSource();
        if (!Loop) {
            _active = false;
            return;
        }
        _virtualOffset = 0.0f;
        // Fall through to re-acquire immediately for a looping sound.
    }

    // ── Virtual path: no live source ─────────────────────────────────────────
    if (_virtualOffset >= _duration) {
        if (Loop) {
            const float d = _duration > 0.0f ? _duration : 1.0f;
            _virtualOffset = std::fmod(_virtualOffset, d);
        } else {
            _active = false;
            return;
        }
    }

    TryAcquire();
    if (!_source) return;

    alGetError();
    const ALuint sampleOffset =
        static_cast<ALuint>(_virtualOffset * static_cast<float>(_bufferData.sampleRate));
    alSourcei(_source, AL_SAMPLE_OFFSET, sampleOffset);

    if (ALenum err = alGetError(); err != AL_NO_ERROR)
        Logger::Warning("[SoundInstance] AL_SAMPLE_OFFSET error: 0x%x", err);

    UpdateSourceParams();
    alSourcePlay(_source);
}
