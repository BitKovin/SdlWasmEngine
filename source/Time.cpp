#include "Time.hpp"

// Define static members
double Time::DeltaTime = 0.0;
float Time::DeltaTimeF = 0.0F;
double Time::GameTime = 0.0;
double Time::GameTimeNoPause = 0.0;
bool Time::GamePaused = false;
Uint64 Time::lastCounter = 0;
double Time::frequency = 0.0;