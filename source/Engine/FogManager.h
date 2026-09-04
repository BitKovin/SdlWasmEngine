#pragma once

#include "glm.h"

class FogManager
{
public:
	
	static inline float StartDistance = -10;
	static inline float EndDistance = 70;
	static inline float Opacity = 0;
	static inline vec3 Color = vec3(0.8f, 0.8f, 0.8f);

	static void Reset()
	{
		StartDistance = -10;
		EndDistance = 70;
		Opacity = 0;
		Color = vec3(0.8f, 0.8f, 0.8f);
	}

private:

};

