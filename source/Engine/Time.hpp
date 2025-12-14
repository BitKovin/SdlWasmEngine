#pragma once
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>
#include <vector>
#include <string>


typedef struct {
    float remainingDuration;   // in seconds (unscaled)
    float totalDuration;       // full duration (including blends)
    float timeScale;           // target multiplier
    float blendIn;             // duration of blend in seconds
    float blendOut;            // duration of blend out seconds
    bool affectSound;          // whether this effect applies to sound
    std::string key;
} TimeScaleEffect;


class Time {
public:
    static double DeltaTime;           // scaled delta time (seconds)
    static float DeltaTimeF;           // scaled delta time (seconds) (float)
    static double DeltaTimeNoTimeScale;           // scaled delta time (seconds)
    static float DeltaTimeFNoTimeScale;           // scaled delta time (seconds) (float)
    static double GameTime;            // total scaled game time (seconds)
    static double GameTimeNoPause;     // total unscaled game time (seconds)
    static float TimeScale;            // global time scale multiplier


    // Call at startup to initialize timing
    static void Init();
    // Call each frame to update timers and apply time scale effects
    static void Update();
    // Add a custom timescale effect (duration in seconds, multiplier, affectSound)
    static void AddTimeScaleEffect(float duration, float scale,
        bool affectSound, const std::string& key,
        float blendIn =0.0f, float blendOut = 0.0f);

    // Get the current compounded timescale (for gameplay)
    static float GetFinalTimeScale();
    // Get the current compounded timescale for sound processing
    static float GetSoundFinalTimeScale();

    static void SetTargetFrameRate(double frameRate);

private:
    static Uint64 lastCounter;          // previous frame's performance counter
    static double frequency;            // performance counter frequency
    static Uint64 frameStartCounter;
    static double frameError;        // drift compensation

    static double TargetFrameRate;   // e.g. 60.0
    static double TargetFrameTime;   // 1 / TargetFrameRate

    // Active time scale effects
    static std::vector<TimeScaleEffect> timeScaleEffects;
};  