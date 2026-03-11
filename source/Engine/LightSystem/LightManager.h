#pragma once

#include "../glm.h"

#include <vector>

#include <Shader.hpp>

class ShaderProgram;

class LightManager
{
public:

	struct PointLightInfo
	{
		vec3 position;
		vec3 color;
		float radius;
		vec3 direction;
		float innerConeAngleDegrees;
		float outerConeAngleDegrees;
	};

	LightManager();
	~LightManager();

	static void Update();
	static void UpdateLightSource(PointLightInfo light);

	static void ApplyPointLightToShader(Shader* shader, vec3 boundsMin, vec3 boundsMax);

	static vec3 LightDirection;

	static float LightDistance;

	static float LightDistance1;
	static float LightDistance2;
	static float LightDistance3;
	static float LightDistance4;

	static float LightRadius1;
	static float LightRadius2;
	static float LightRadius3;
	static float LightRadius4;

	static int ShadowMapResolution;
	static float LightDistanceMultiplier;

	static float ShaddowOffsetScale;

	static mat4 lightView1;
	static mat4 lightProjection1;

	static mat4 lightView2;
	static mat4 lightProjection2;

	static mat4 lightView3;
	static mat4 lightProjection3;

	static mat4 lightView4;
	static mat4 lightProjection4;

	static bool DirectionalShadowsEnabled;

private:

	static void CalculateLightMatrices(
		float lightDistance, glm::vec3 cameraPos,
		glm::mat4& outLightView,
		glm::mat4& outLightProjection
	);

	static void FinishPointLights();

	static inline std::vector<PointLightInfo> m_lights;
	static inline std::vector<PointLightInfo> m_finalLights;

};

