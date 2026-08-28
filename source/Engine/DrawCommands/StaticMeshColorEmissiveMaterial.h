#pragma once

#include "../glm.h"
#include "SurfaceType.h"
#include "../Texture.hpp"
#include "../ShaderManager.h"
#include "../Shader.hpp"

// What StaticMesh::DrawForward/DrawDepth/DrawCustomId used to read directly off StaticMesh, as plain data copied into whichever StaticMeshDrawCommand owns it.
struct StaticMeshColorEmissiveMaterial
{
	Texture* ColorTexture = nullptr;
	Texture* EmissiveTexture = nullptr;

	// Raw bgfx handle fallback for callers binding by native id instead of a Texture* - EffectiveSurfaceType() can't see alpha data through these.
	uint32_t ColorTextureId = 0;
	uint32_t EmissiveTextureId = 0;

	vec4 Color = vec4(1.0f);

	float ViewmodelScaleFactor = 1.0f;

	int CustomId = 0;

	// Explicit override: if set, EffectiveSurfaceType() returns this directly, skipping detection entirely. Needed whenever transparency can't be auto-detected - e.g. a raw ColorTextureId with no Texture* to inspect (see UiBilboard).
	SurfaceType SurfaceTypeOverride = SurfaceType::NONE;

	// Explicit override: treat as a hard alpha-tested cutout regardless of detected transparency - no data signal distinguishes "cutout" from "blended".
	bool Masked = false;

	static constexpr float kAlphaDiscardThreshold = 0.999f;

	// SurfaceTypeOverride wins outright; else Masked wins; else tint alpha < 1 or a real texture alpha channel -> Transparent; else Opaque.
	SurfaceType EffectiveSurfaceType() const
	{
		if (SurfaceTypeOverride != SurfaceType::NONE) return SurfaceTypeOverride;
		if (Masked) return SurfaceType::Masked;

		bool textureHasAlpha = ColorTexture && ColorTexture->transparent;

		if (Color.a < 1.0f || textureHasAlpha) return SurfaceType::Transparent;

		return SurfaceType::Opaque;
	}

	bool NeedsAlphaDiscard() const
	{
		SurfaceType t = EffectiveSurfaceType();
		return t == SurfaceType::Masked || t == SurfaceType::Transparent;
	}

	// Forward pass always uses the same shader family - "masked" is a uniform, not a shader switch.
	Shader* GetForwardShader(const std::string& pixelShader) const
	{
		return ShaderManager::GetShaderProgram("vs_default", pixelShader);
	}

	// Depth pass does select by name: empty shader unless alpha testing is needed.
	Shader* GetDepthShader() const
	{
		return ShaderManager::GetShaderProgram("vs_default", NeedsAlphaDiscard() ? "fs_mask" : "fs_empty");
	}

	void BindColorTexture(Shader* shader) const
	{
		if (ColorTexture)
			shader->SetTexture("u_texture", ColorTexture);
		else
			shader->SetTexture("u_texture", ColorTextureId);
	}

	void BindEmissiveTexture(Shader* shader) const
	{
		if (EmissiveTexture)
			shader->SetTexture("u_textureEmissive", EmissiveTexture);
		else
			shader->SetTexture("u_textureEmissive", EmissiveTextureId);
	}
};
