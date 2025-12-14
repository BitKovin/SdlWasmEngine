#include "Time.hpp"

#include "EngineMain.h"

// static member definitions
double Time::DeltaTime = 0.0;
float Time::DeltaTimeF = 0.0f;
double Time::DeltaTimeNoTimeScale = 0.0;
float Time::DeltaTimeFNoTimeScale = 0.0f;
double Time::GameTime = 0.0;
double Time::GameTimeNoPause = 0.0;
float Time::TimeScale = 1.0f;
Uint64 Time::lastCounter = 0;
double Time::frequency = 0.0;
std::vector<TimeScaleEffect> Time::timeScaleEffects;

double Time::TargetFrameRate = 6000.0;
double Time::TargetFrameTime = 1.0 / 6000.0;
double Time::frameError = 0.0;
Uint64 Time::frameStartCounter = 0;


void Time::Init() {
    frequency = static_cast<double>(SDL_GetPerformanceFrequency());
    lastCounter = SDL_GetPerformanceCounter();
    DeltaTime = 0.0;
}

void Time::Update()
{
    const Uint64 freq = SDL_GetPerformanceFrequency();

    // ----------------------------------------
    // Frame limiter (runs BEFORE delta compute)
    // ----------------------------------------
    if (TargetFrameRate > 0.0)
    {
        const double targetFrameTime = TargetFrameTime;

        Uint64 now = SDL_GetPerformanceCounter();
        double elapsed =
            (double)(now - frameStartCounter) / freq;

        double remaining =
            targetFrameTime - elapsed - frameError;

        // ---- Coarse sleep (milliseconds)
        if (remaining > 0.002) // sleep only if >2ms
        {
            SDL_Delay((Uint32)((remaining - 0.001) * 1000.0));
        }

        // ---- Fine spin wait (sub-millisecond)
        while (true)
        {
            now = SDL_GetPerformanceCounter();
            elapsed =
                (double)(now - frameStartCounter) / freq;

            if (elapsed >= targetFrameTime)
                break;
        }

        // ---- Error compensation (prevents drift)
        frameError = elapsed - targetFrameTime;
        if (frameError > 0.002)
            frameError = 0.002; // clamp runaway error
    }

    frameStartCounter = SDL_GetPerformanceCounter();

    // ----------------------------------------
    // Delta time calculation
    // ----------------------------------------
    Uint64 currentCounter = frameStartCounter;
    double rawDelta =
        (double)(currentCounter - lastCounter) / freq;
    lastCounter = currentCounter;

    // Clamp large spikes
    if (rawDelta > 0.1)
        rawDelta = 0.1;

    // ----------------------------------------
    // Time scale effects (unscaled)
    // ----------------------------------------
    for (auto it = timeScaleEffects.begin();
        it != timeScaleEffects.end();)
    {
        it->remainingDuration -= rawDelta;
        if (it->remainingDuration <= 0.0)
            it = timeScaleEffects.erase(it);
        else
            ++it;
    }

    DeltaTimeNoTimeScale = rawDelta;
    DeltaTimeFNoTimeScale = (float)rawDelta;

    float finalScale = GetFinalTimeScale();

    double scaledDelta = rawDelta * finalScale;
    DeltaTime = scaledDelta;
    DeltaTimeF = (float)scaledDelta;

    GameTimeNoPause += rawDelta;
    if (!EngineMain::MainInstance->Paused)
        GameTime += scaledDelta;
}


void Time::AddTimeScaleEffect(float duration, float scale,
    bool affectSound, const std::string& key,
    float blendIn /*=0.0f*/, float blendOut /*=0.0f*/)
{
    // Remove existing effects with same key if applicable
    if (!key.empty()) {
        timeScaleEffects.erase(
            std::remove_if(
                timeScaleEffects.begin(),
                timeScaleEffects.end(),
                [&](const TimeScaleEffect& e) {
                    return e.key == key;
                }
            ),
            timeScaleEffects.end()
        );
    }

    TimeScaleEffect e;
    e.remainingDuration = duration + blendIn + blendOut;
    e.totalDuration = e.remainingDuration;
    e.timeScale = scale;
    e.blendIn = blendIn;
    e.blendOut = blendOut;
    e.affectSound = affectSound;
    e.key = key;

    timeScaleEffects.push_back(e);
}


float Time::GetFinalTimeScale() {
    float result = TimeScale;

    for (const auto& e : timeScaleEffects) {
        float t = e.totalDuration - e.remainingDuration;

        float effectiveScale = e.timeScale;

        // Blend in
        if (t < e.blendIn && e.blendIn > 0.0f) {
            float alpha = t / e.blendIn;
            effectiveScale = 1.0f + (e.timeScale - 1.0f) * alpha;
        }
        // Blend out
        else if (e.remainingDuration < e.blendOut && e.blendOut > 0.0f) {
            float alpha = e.remainingDuration / e.blendOut;
            effectiveScale = 1.0f + (e.timeScale - 1.0f) * alpha;
        }

        result *= effectiveScale;
    }

    return result;
}


float Time::GetSoundFinalTimeScale() {
    float result = TimeScale;

    for (const auto& e : timeScaleEffects) {
        if (!e.affectSound) continue;

        float t = e.totalDuration - e.remainingDuration;
        float effectiveScale = e.timeScale;

        if (t < e.blendIn && e.blendIn > 0.0f) {
            float alpha = t / e.blendIn;
            effectiveScale = 1.0f + (e.timeScale - 1.0f) * alpha;
        }
        else if (e.remainingDuration < e.blendOut && e.blendOut > 0.0f) {
            float alpha = e.remainingDuration / e.blendOut;
            effectiveScale = 1.0f + (e.timeScale - 1.0f) * alpha;
        }

        result *= effectiveScale;
    }

    return result;
}

void Time::SetTargetFrameRate(double frameRate)
{
    TargetFrameRate = frameRate;
    TargetFrameTime = 1.0 / TargetFrameRate;
}

