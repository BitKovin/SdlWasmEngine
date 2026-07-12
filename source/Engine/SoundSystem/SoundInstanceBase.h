#pragma once

#include "../glm.h"


enum class SoundType
{
    SFX,
	Music,
    Voice,
    UI
};

/// Abstract base that exposes the public interface used by the rest of the engine.
/// Concrete playback lives in SoundInstance (OpenAL) or future back-ends.
class SoundInstanceBase
{
public:
    virtual ~SoundInstanceBase() = default;

    virtual void Play()                  {}
    virtual void Stop()                  {}
    virtual void Update(float deltaTime) {}
    virtual bool IsPlaying()             { return false; }

    // ── Playback properties ──────────────────────────────────────────────────
    float     Priority    = 0.0f;  ///< Higher = more important; used when stealing sources.

    bool      Paused      = false;
    bool      Is2D        = false;
    bool      IsUISound   = false;
    bool      Loop        = false;

    float     Volume      = 1.0f;
    float     Pitch       = 1.0f;
    float     MinDistance = 1.0f;
    float     MaxDistance = 20.0f;

    glm::vec3 Position    = { 0.0f, 0.0f, 0.0f };
    glm::vec3 Velocity    = { 0.0f, 0.0f, 0.0f };
    glm::vec3 Direction   = { 0.0f, 0.0f, 1.0f };

	SoundType Type = SoundType::SFX;

    // ── Spatial audio (vaudio) flags ─────────────────────────────────────────
    // These only ever affect the additional occlusion + environment reverb
    // layered on by SpatialSoundManager (see SpatialSound/SpatialSoundManager.h).
    // Any effects authored directly on this instance (EnableFilter/EnableEcho/
    // EnableReverb on SoundInstance, or hand-placed DSPs elsewhere) are never
    // touched by either flag. UI sounds (IsUISound) never get spatial
    // treatment regardless of these flags.

    /// Opts this instance out of vaudio spatial audio entirely.
    bool DisableSpatial = false;

    /// True for ambience/room-tone beds that don't represent a real emitter
    /// position (e.g. wind, distant traffic, a music bed). Skips per-position
    /// occlusion + emitter tracking entirely, but still receives the
    /// listener's ambient reverb — "how it's heard" is controlled by the
    /// spatial audio manager rather than by where this sound is placed.
    bool EnvironmentalSound = false;

protected:
    virtual bool  IsGamePaused()   const { return false; }
    virtual float GetPitchScale()  const { return 1.0f;  }
    virtual float GetFinalVolume() const { return 1.0f;  }
};
