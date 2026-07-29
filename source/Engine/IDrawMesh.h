#pragma once

#include "EObject.hpp"

#include <vector>

#include "glm.h"

#include "FrustrumCull.hpp"
#include "BoundingBox.hpp"
#include "Camera.h"

#include "MeshUtils.hpp"

using namespace std;

class IDrawMesh : public EObject
{

public:

	bool IsViewmodel = false;

	bool ReceiveDetailShadows = false;

	bool Transparent = false;

	bool OnlyShadows = false;

	bool StaticNavigation = false;

	double LastRenderedTime = 0;
	bool WasRended = true;

	virtual ~IDrawMesh() = default;

	virtual float GetDistanceToCamera()
	{
		return 1;
	}

	virtual vector<MeshUtils::PositionVerticesIndices> GetNavObstacleMeshes() { return vector<MeshUtils::PositionVerticesIndices>(); }


	virtual void DrawForward(mat4x4 view, mat4x4 projection) {}

	virtual void DrawDepth(mat4x4 view, mat4x4 projection) {}
	virtual void DrawCustomId(mat4x4 view, mat4x4 projection) {}

	virtual void DrawShadow(mat4x4 view, mat4x4 projection) {}

	virtual void DrawMeshShadow(mat4x4 view, mat4x4 projection) {}

	virtual void FinalizeFrameData(){}

	//called in parallel from game thread right at the end of a frame. Used to finalize non GPU data
	virtual void PreFinalize(){}

	virtual void PreDraw(){}

	virtual bool IsCameraVisible() { return IsInFrustrum(Camera::frustum); }
	virtual bool IsShadowVisible() { return true; }
	virtual bool IsDetailShadow() { return false; }

	virtual bool IsInFrustrum(Frustum frustrum) { return true; };

	//called when object wasn't rendered in previous frame
	virtual void StartedRendering(){}

	virtual void RemoveFromLevel(){}

	virtual void PreloadAssets(){}

	virtual BoundingBox GetBoundingBox() { return BoundingBox(); }

};