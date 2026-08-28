#pragma once

#include "../glm.h"
#include "SurfaceType.h"
#include "../BoundingBox.hpp"

// Self-contained thing the renderer can draw - geometry + material + transform, no knowledge of the IDrawMesh/Entity that produced it.
// Hard rule: nothing reachable from Draw*() may touch an IDrawMesh/Entity pointer or callback into one - Draw*() may run on a separate render thread. All data must be computed in the owning mesh's PreFinalize() and committed here in FinalizeFrameData() (also the only place to create/manage bgfx buffers). A handle/pointer into already-resolved read-only data owned elsewhere (bgfx handle, Texture*, a mesh's own finalized bone array) is fine.
class IDrawCommand
{

public:

	virtual ~IDrawCommand() = default;

	// Copied from the owning IDrawMesh every frame in FinalizeFrameData().
	mat4 WorldMatrix = mat4(1);
	BoundingBox WorldBounds;

	bool IsViewmodel          = false;
	bool OnlyShadows          = false;
	bool ReceiveDetailShadows = false;

	// Set by Level::FinalizeFrame(), consumed by the opaque/transparent sort.
	float SortDistance = 0;

	virtual SurfaceType GetSurfaceType() const { return SurfaceType::Opaque; }

	// Clusters commands sharing a shader/material during the opaque sort - doesn't need to be globally unique.
	virtual uint64_t GetSortKey() const { return 0; }

	virtual void DrawForward(const mat4& view, const mat4& projection) = 0;
	virtual void DrawDepth(const mat4& view, const mat4& projection) {}
	virtual void DrawCustomId(const mat4& view, const mat4& projection) {}

	// Reserved for directional-light shadow-map cascades - unused, RenderDirectionalLightShadows() is still an empty stub.
	virtual void DrawShadow(const mat4& view, const mat4& projection) {}

	// Detail (stencil volume) shadows - base no-op is correct for anything that doesn't cast one.
	virtual void DrawShadowVolumeStencil(const mat4& view, const mat4& projection) {}
	virtual vec3 GetShadowColorMult() const { return vec3(1.0f); }

};
