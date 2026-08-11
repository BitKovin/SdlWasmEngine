#include "PointLight.h"
#include "../LightSystem/LightManager.h"

PointLight::PointLight()
{
}

PointLight::~PointLight()
{
}

void PointLight::Finalize()
{
	if (Visible == false) return;

	LightManager::UpdateLightSource(LightManager::PointLightInfo{ 
		Position,color*intensity, radius, 
		MathHelper::GetForwardVector(Rotation),
		innerConeAngleDegrees, outerConeAngleDegrees });

}