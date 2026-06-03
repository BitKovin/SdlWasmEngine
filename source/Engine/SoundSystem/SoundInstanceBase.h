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

protected:
    virtual bool  IsGamePaused()   const { return false; }
    virtual float GetPitchScale()  const { return 1.0f;  }
    virtual float GetFinalVolume() const { return 1.0f;  }
};
