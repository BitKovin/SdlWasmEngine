#pragma once

#include <glm.h>
#include <algorithm>
#include <cmath>

#include <BSP/Quake3Bsp.h>

namespace LightVisibility
{
    // === Tuning parameters ===
    // How fast visibility rises with light (stealth feel control)
    inline float PerceptualK = 1.2f;

    // Optional bias if your light probes are very dim/bright
    inline float BrightnessScale = 7.0f;

    // =========================

    inline float Luminance(const glm::vec3& c)
    {
        
        return std::max(std::max(c.x,c.y),c.z);
    }

    inline float PerceptualCurve(float x)
    {

		if (x <= 0.005f) return 0.0f;

        // Converts linear light into perceived visibility
        return 1.0f - std::exp(-x * PerceptualK);
    }

    inline float Compute(const LightVolPointData& data)
    {
        glm::vec3 totalLight = data.directColor + data.ambientColor;

        float brightness = (Luminance(totalLight) - 0.05f) * BrightnessScale;

        float visibility = PerceptualCurve(brightness);
		//visibility = powf(visibility, 1.5f);

        if (visibility < 0.05)
            return 0;

        return std::clamp(visibility, 0.0f, 1.0f);
    }
}
