#pragma once

#include "EObject.hpp"

#include <vector>

#include "glm.h"

#include "FrustrumCull.hpp"
#include "BoundingBox.hpp"
#include "Camera.h"

#include "MeshUtils.hpp"

#include "DrawCommands/IDrawCommand.h"

using namespace std;

class IDrawMesh : public EObject
{

public:

	bool IsViewmodel = false;

	bool ReceiveDetailShadows = false;

	// No longer read by Renderer - see SurfaceType.h. Left in place, doing nothing, so existing reads/writes of mesh->Transparent still compile.
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

	// Replaces DrawForward/DrawDepth/DrawCustomId/DrawShadow. Called once per visible mesh per frame to gather this mesh's persistent IDrawCommand(s) - never allocate a new command here.
	virtual void CollectDrawCommands(vector<IDrawCommand*>& outCommands) {}

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
