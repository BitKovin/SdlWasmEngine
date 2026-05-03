#include "SoundInstance.hpp"

#include "../EngineMain.h"
#include "SoundManager.hpp"
#include <Logger.hpp>

#define SOUND_POOL_DEBUG

void SoundInstance::SourcePool::Init()
{
    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
    if (initialized) return;
    initialized = true;

    // Clear any existing error state
    alGetError();

    // Try OpenAL Soft hints
    ALCdevice* dev = alcGetContextsDevice(alcGetCurrentContext());
    if (dev) {
        alcGetIntegerv(dev, ALC_MONO_SOURCES, 1, (ALCint*)&maxMono);
        alcGetIntegerv(dev, ALC_STEREO_SOURCES, 1, (ALCint*)&maxStereo);
        Logger::Info("[SourcePool] Device hints: maxMono=%zu, maxStereo=%zu", maxMono, maxStereo);
    }

    if (maxMono > 100000)
        maxMono = 128;

    if(maxStereo > 100000)
        maxStereo = 32;

    Logger::Info("max mono: %zu", maxMono);
    Logger::Info("max stereo: %zu", maxStereo);

    // Ensure at least a minimum limit
    if (maxMono < 8) maxMono = 8;  // Arbitrary safe minimum
    if (maxStereo == 0) maxStereo = maxMono;  // Default stereo to mono limit
}

ALuint SoundInstance::SourcePool::Acquire(bool stereo, SoundInstance* requester)
{
    alGetError();
    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);

    alcMakeContextCurrent(requester->_bufferData.context);

    auto& pool = stereo ? freeStereo : freeMono;
    auto& alloc = stereo ? allocatedStereo : allocatedMono;
    size_t limit = stereo ? maxStereo : maxMono;
    size_t used = alloc - pool.size();

    // 1) Reuse
    if (!pool.empty()) {
        ALuint s = pool.back(); pool.pop_back();
        liveOwners[s] = { requester, ++globalTimestamp };
        return s;
    }

    // 2) Gen new if under limit
    if (used < limit) {
        ALuint s = 0;
        alGenSources(1, &s);
        if (alGetError() == AL_NO_ERROR) {
            ++alloc;
            liveOwners[s] = { requester, ++globalTimestamp };
            return s;
        }
    }

    if (requester->_isStereo) return 0;

    // 3) Steal oldest lowest-prio non-looping
    ALuint victimSrc = 0;
    float  victimPrio = std::numeric_limits<float>::infinity();
    uint64_t victimTime = UINT64_MAX;

    for (auto& [src, info] : liveOwners) 
    {
        auto* inst = info.inst;
        if (inst->_isStereo) continue;       // skip stereo
        if (inst->Loop) continue;            // skip looping
        float pr = inst->Priority;
        uint64_t ts = info.timestamp;
        // choose if lower priority, or equal pr
        if (pr < victimPrio ||
            (pr == victimPrio))
        {
            victimPrio = pr;
            victimTime = ts;
            victimSrc = src;
        }
    }

    if (victimSrc) {
        // steal it
        alSourceStop(victimSrc);
        liveOwners.erase(victimSrc);
        liveOwners[victimSrc] = { requester, ++globalTimestamp };
        return victimSrc;
    }

    // 4) none to steal
    return 0;
}

void SoundInstance::SourcePool::Release(ALuint src, bool stereo, SoundInstance* inst)
{


    alcMakeContextCurrent(inst->_bufferData.context);

    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);
    alSourceStop(src);
    alSourcei(src, AL_BUFFER, 0);
    liveOwners.erase(src);
    auto& pool = stereo ? freeStereo : freeMono;
    
    pool.push_back(src);

#ifdef SOUND_POOL_DEBUG
    std::cerr << "[SourcePool::Release] Released source " << src
        << " back to " << (stereo ? "stereo" : "mono") << " pool\n";
#endif
}

bool SoundInstance::IsGamePaused() const
{
    return EngineMain::MainInstance->Paused;
}

float SoundInstance::GetPitchScale() const
{

    if (IsUISound)
        return 1.0f;

    return Time::GetSoundFinalTimeScale();
}

float SoundInstance::GetFinalVolume() const
{
    return Volume * SoundManager::GlobalVolume;
}

void SoundInstance::UpdateSourceParams()
{
    if (!_source) return;

    alcMakeContextCurrent(this->_bufferData.context);

    // global attenuation off
#ifdef AL_DISTANCE_MODEL_NONE
    alDistanceModel(AL_DISTANCE_MODEL_NONE);
#else
    alDistanceModel(0xD000);
#endif

    // looping & pitch
    alSourcei(_source, AL_LOOPING, Loop ? AL_TRUE : AL_FALSE);
    alSourcef(_source, AL_PITCH, Pitch * GetPitchScale());

    // spatial vs 2D
    if (_isStereo || Is2D || IsUISound) {
        // head-relative, no distance attenuation
        alSourcei(_source, AL_SOURCE_RELATIVE, AL_TRUE);
        alSourcef(_source, AL_ROLLOFF_FACTOR, 0.0f);

        // 2) raw gain only
        alSourcef(_source, AL_GAIN, GetFinalVolume());



        // 5) clear any leftover position/orientation
        alSource3f(_source, AL_POSITION, 0, 0, 0);
        alSource3f(_source, AL_VELOCITY, 0, 0, 0);

    }
    else {
        alSourcei(_source, AL_SOURCE_RELATIVE, AL_FALSE);
        alSource3f(_source, AL_POSITION, Position.x, Position.y, Position.z);
        alSource3f(_source, AL_VELOCITY, Velocity.x, Velocity.y, Velocity.z);
        alSource3f(_source, AL_DIRECTION, Direction.x, Direction.y, Direction.z);
        alSourcef(_source, AL_CONE_INNER_ANGLE, ConeInnerAngle);
        alSourcef(_source, AL_CONE_OUTER_ANGLE, ConeOuterAngle);
        alSourcef(_source, AL_CONE_OUTER_GAIN, ConeOuterGain);

        float gain = ComputeDistanceGain();
        alSourcef(_source, AL_GAIN, gain);
    }

#ifndef DISABLE_EFX
    EnsureEFX();
    if (EnableFilter && _filter) ApplyFilter();
    if (EnableEcho && _slotEcho)   alSource3i(_source, AL_AUXILIARY_SEND_FILTER, _slotEcho, 0, AL_FILTER_NULL);
    if (EnableReverb && _slotReverb) alSource3i(_source, AL_AUXILIARY_SEND_FILTER, _slotReverb, 1, AL_FILTER_NULL);
#endif
}

void SoundInstance::Play()
{
    Paused = false; // Ensure paused is off when playing
    _virtualOffset = 0;

    if (_active && _source != 0) {
        return;
    }

    _active = true;
    TryAcquire();
    if (_source) {
        UpdateSourceParams();
        alSourcePlay(_source);
    }
}

void SoundInstance::Stop()
{
    std::lock_guard<std::recursive_mutex> lock(SoundManager::audioMutex);

    alcMakeContextCurrent(this->_bufferData.context);

    if (_source) {
        alSourceStop(_source);
        ReleaseSource();
    }
    _virtualOffset = 0.0f;
    _active = false;
}

void SoundInstance::Update(float deltaTime)
{
    SourcePool::globalTimestamp = Time::GameTimeNoPause;

    if (Paused || (IsGamePaused() && IsUISound == false)) {
        if (_source) {
            // Capture current playback position if possible
            //alGetSourcef(_source, AL_SEC_OFFSET, &_virtualOffset);
            alSourceStop(_source);
            ReleaseSource();
        }
        return;
    }
    else
    {
        _virtualOffset += deltaTime;
    }

    if (!_active) return;

    if (_source) 
    {
        UpdateSourceParams();
        ALint state;
        alGetSourcei(_source, AL_SOURCE_STATE, &state);
        if (state == AL_STOPPED) {
            ReleaseSource();
            _active = Loop;
            if (_active) _virtualOffset = 0.0f;
        }
    }
    else {
        // Only advance virtual time if not paused
        
        if (_virtualOffset >= _duration) {
            if (Loop) {
                _virtualOffset = fmod(_virtualOffset, _duration);
            }
            else {
                _active = false;
            }
        }
        TryAcquire();
        if (_source && _active) {
            
            alGetError();

            alcMakeContextCurrent(this->_bufferData.context);

            ALuint sampleOffset = _virtualOffset * _bufferData.sampleRate;
            alSourcei(_source, AL_SAMPLE_OFFSET, sampleOffset);

            ALenum error = alGetError();
            if (error != AL_NO_ERROR) {
                std::cerr << "OpenAL error: " << error << std::endl;
            }

            UpdateSourceParams();
            alSourcePlay(_source);
        }
    }
}
