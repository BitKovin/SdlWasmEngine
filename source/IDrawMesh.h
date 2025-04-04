#pragma once

#include "EObject.hpp"

#include "glm.h"

class IDrawMesh : public EObject
{

public:

	virtual void DrawForward(mat4x4 view, mat4x4 projection) {}

	virtual void DrawDepth(mat4x4 view, mat4x4 projection) {}

	virtual void DrawShadow(mat4x4 view, mat4x4 projection) {}

	virtual void FinalizeFrameData(){}

	virtual bool IsCameraVisible() { return true; }
	virtual bool IsShadowVisible() { return true; }

};