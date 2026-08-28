#pragma once

#include "../glm.h"

// The two full-screen stencil operations that bookend a detail-shadow group in Renderer::DrawDetailShadows - moved off StaticMesh so Renderer doesn't need to include it.
namespace DetailShadowUtils
{
	// Multiply the framebuffer by shadowColor wherever stencil != 0.
	void ApplyShadowDarkening(const vec3& shadowColor);

	// Reset the stencil buffer back to 0 wherever it was marked.
	void ClearShadowStencil();
}
