#pragma once

#include "EObject.hpp"

#include "glm.h"

#include "FrustrumCull.hpp"
#include "Camera.h"

class IDrawMesh : public EObject
{

public:

	bool IsViewmodel = false;

	bool Transparent = false;

	virtual float GetDistanceToCamera()
	{
		return 1;
	}

	virtual void DrawForward(mat4x4 view, mat4x4 projection) {}

	virtual void DrawDepth(mat4x4 view, mat4x4 projection) {}

	virtual void DrawShadow(mat4x4 view, mat4x4 projection) {}

	virtual void FinalizeFrameData(){}

	virtual bool IsCameraVisible() { return IsInFrustrum(Camera::frustum); }
	virtual bool IsShadowVisible() { return true; }

	virtual bool IsInFrustrum(Frustum frustrum) { return true; };

};