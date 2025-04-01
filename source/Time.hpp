#pragma once
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>

class Time {
public:
    static double DeltaTime;       // Time between the last two frames in seconds.
    static float DeltaTimeF;       // Time between the last two frames in seconds.
    static double GameTime;       // Time between the last two frames in seconds.
private:
    static Uint64 lastCounter;     // Stores the previous frame's counter.
    static double frequency;       // Stores the counter frequency.

public:
    // Call this once at the start to initialize the timer.
    static void Init() {
        frequency = static_cast<double>(SDL_GetPerformanceFrequency());
        lastCounter = SDL_GetPerformanceCounter();
        DeltaTime = 0.0;
    }

    // Call this every frame to update DeltaTime.
    static void Update() {
        Uint64 currentCounter = SDL_GetPerformanceCounter();
        DeltaTime = (currentCounter - lastCounter) / frequency;
        lastCounter = currentCounter;

        if (DeltaTime > 0.1)
            DeltaTime = 0.1;

        GameTime += DeltaTime;

        DeltaTimeF = (float)DeltaTime;

    }
};