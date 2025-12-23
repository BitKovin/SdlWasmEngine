#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <string>

struct TimeScaleEffect {
    float remainingDuration;
    float totalDuration;
    float timeScale;
    float blendIn;
    float blendOut;
    bool affectSound;
    std::string key;
};

class Time {
public:
    static double DeltaTime;
    static float DeltaTimeF;
    static double DeltaTimeNoTimeScale;
    static float DeltaTimeFNoTimeScale;
    static double GameTime;
    static double GameTimeNoPause;
    static float TimeScale;

    static double TargetFrameRate;
    static double TargetFrameTime;

	static bool SimulationLikeFixedTimeStep;

    static void Init();
    static void Update();
    static void AddTimeScaleEffect(float duration, float scale,
        bool affectSound, const std::string& key,
        float blendIn = 0.0f, float blendOut = 0.0f);
    static float GetFinalTimeScale();
    static float GetSoundFinalTimeScale();
    static void SetTargetFrameRate(double frameRate);

private:
    static double frequency;
    static std::vector<TimeScaleEffect> timeScaleEffects;
    static double frameError;

    // New for accurate limiter
    static double desiredTime;
    static double frameStartTime;
    static double lastFrameStartTime;

    static void LimitFrameRate();
};