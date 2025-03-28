#pragma once

#include "EObject.hpp"

#include "glm.h"

class IDrawMesh : EObject
{

public:

	 virtual void DrawForward(mat4x4 viewProjection);

	 virtual void DrawDepth(mat4x4 viewProjection);

	 virtual void DrawShadow(mat4x4 viewProjection);

};