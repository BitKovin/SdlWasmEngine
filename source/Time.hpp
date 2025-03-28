#pragma once
#include <SDL2/SDL_stdinc.h>
#include <SDL2/SDL_timer.h>

class Time
{
public:
	static float DeltaTime;

	static void Update()
	{
		Uint64 NOW = SDL_GetPerformanceCounter();

		DeltaTime = (NOW - LAST) / (double)SDL_GetPerformanceFrequency();

		LAST = NOW;

	}

private:

	static Uint64 LAST;

};

float Time::DeltaTime = 0;
Uint64 Time::LAST = Uint64();