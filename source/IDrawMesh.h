#pragma once

#include "EObject.hpp"

#include <vector>

#include "glm.h"

#include "FrustrumCull.hpp"
#include "Camera.h"

#include "MeshUtils.hpp"

using namespace std;

class IDrawMesh : public EObject
{

public:

	bool IsViewmodel = false;

	bool Transparent = false;

	bool StaticNavigation = false;

	virtual ~IDrawMesh() {}

	virtual float GetDistanceToCamera()
	{
		return 1;
	}

	virtual vector<MeshUtils::PositionVerticesIndices> GetNavObstacleMeshes() { return vector<MeshUtils::PositionVerticesIndices>(); }


	virtual void DrawForward(mat4x4 view, mat4x4 projection) {}

	virtual void DrawDepth(mat4x4 view, mat4x4 projection) {}

	virtual void DrawShadow(mat4x4 view, mat4x4 projection) {}

	virtual void FinalizeFrameData(){}

	virtual bool IsCameraVisible() { return IsInFrustrum(Camera::frustum); }
	virtual bool IsShadowVisible() { return true; }

	virtual bool IsInFrustrum(Frustum frustrum) { return true; };

	virtual void RemoveFromLevel(){}

};