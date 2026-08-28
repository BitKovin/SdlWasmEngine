#pragma once

#include "../DrawCommands/IDrawCommand.h"


// BSPModelRef's command. GetSurfaceType() always returns Opaque, never reads the now-inert BSPModelRef::Transparent - bsp->RenderBSP() already queues transparent faces unconditionally, and Renderer flushes that queue unconditionally too, so the old Transparent==true path only ever hurt (it disabled depth-write/pre-pass for the submodel's opaque faces for no benefit). Doesn't cast/receive detail shadows, matching BSPModelRef today.
class BSPModelDrawCommand : public IDrawCommand
{

public:

	class CQuake3BSP* bsp = nullptr;
	int id = -1;
	class tBSPModel* model = nullptr;

	bool useBspVisibility = false;
	bool IsStatic = true;

	SurfaceType GetSurfaceType() const override { return SurfaceType::Opaque; }

	void DrawForward(const mat4& view, const mat4& projection) override;
	void DrawDepth(const mat4& view, const mat4& projection) override;

};
