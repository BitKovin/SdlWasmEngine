#pragma once

#include <AL/al.h>
#include <AL/alc.h>

/// Lightweight handle to a decoded WAV buffer, shared across SoundInstance copies.
/// Mono buffers belong to SoundManager::contextMono  (HRTF-enabled, 3D).
/// Stereo buffers belong to SoundManager::contextStereo (flat stereo, 2D/UI).
struct SoundBufferData
{
    ALCcontext* context    = nullptr;
    bool        stereo     = false;
    ALuint      buffer     = 0;
    float       duration   = 0.0f;   ///< Clip length in seconds.
    ALuint      sampleRate = 0;

    bool IsValid() const { return buffer != 0 && context != nullptr; }
};
