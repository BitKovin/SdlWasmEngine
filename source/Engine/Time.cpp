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
double Time::frequency = 0.0;
std::vector<TimeScaleEffect> Time::timeScaleEffects;
double Time::TargetFrameRate = 60.0;
double Time::TargetFrameTime = 1.0 / 60.0;
double Time::frameError = 0.0;
double Time::desiredTime = 0.0;
double Time::frameStartTime = 0.0;
double Time::lastFrameStartTime = 0.0;

void Time::Init() {
    frequency = 1.0 / static_cast<double>(SDL_GetPerformanceFrequency());
    Uint64 initialCounter = SDL_GetPerformanceCounter();
    lastFrameStartTime = static_cast<double>(initialCounter) * frequency;
    desiredTime = lastFrameStartTime + TargetFrameTime; // Schedule first frame
    DeltaTime = 0.0;
}

void Time::LimitFrameRate() {
    if (TargetFrameRate > 0.0) {
        const double targetFrameTime = TargetFrameTime;
        Uint64 nowCounter = SDL_GetPerformanceCounter();
        double currentTime = static_cast<double>(nowCounter) * frequency;
        double remaining = desiredTime - currentTime;
        // ---- Coarse sleep (milliseconds)
        if (remaining > 0.002) { // sleep only if >2ms
            SDL_Delay(static_cast<Uint32>((remaining - 0.002) * 1000.0));
        }
        // ---- Fine spin wait (sub-millisecond)
        while (true) {
            nowCounter = SDL_GetPerformanceCounter();
            currentTime = static_cast<double>(nowCounter) * frequency;
            if (currentTime >= desiredTime)
                break;
        }
        // ---- Error compensation (prevents drift)
        frameError = currentTime - desiredTime;
        if (frameError > 0.002)
            frameError = 0.002; // clamp runaway error

        // Update for next frame
        desiredTime += targetFrameTime;

        // Set frame start
        frameStartTime = currentTime;
    }
    else {
        Uint64 nowCounter = SDL_GetPerformanceCounter();
        frameStartTime = static_cast<double>(nowCounter) * frequency;
    }
}

void Time::Update() {
    LimitFrameRate();

    // ----------------------------------------
    // Delta time calculation
    // ----------------------------------------
    double rawDelta = frameStartTime - lastFrameStartTime;
    lastFrameStartTime = frameStartTime;
    // Clamp large spikes
    if (rawDelta > 0.1)
        rawDelta = 0.1;
    // ----------------------------------------
    // Time scale effects (unscaled)
    // ----------------------------------------
    for (auto it = timeScaleEffects.begin();
        it != timeScaleEffects.end();) {
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
    float blendIn /*=0.0f*/, float blendOut /*=0.0f*/) {
    // Remove existing effects with same key if applicable
    if (!key.empty()) {
        timeScaleEffects.erase(
            std::remove_if(
                timeScaleEffects.begin(),
                timeScaleEffects.end(),
                [&](const TimeScaleEffect& e) { return e.key == key; }),
            timeScaleEffects.end());
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

void Time::SetTargetFrameRate(double frameRate) {
    TargetFrameRate = frameRate;
    TargetFrameTime = 1.0 / TargetFrameRate;
}