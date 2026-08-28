#pragma once

#include "IDrawCommand.h"
#include "StaticMeshColorEmissiveMaterial.h"

#include <bgfx/bgfx.h>
#include <BgfxStateManager.h>
#include <string>
#include <map>

class Shader;

// One submesh of one StaticMesh instance (one per entry in model->meshes) - each gets its own Material and derived SurfaceType, so opaque and transparent submeshes of the same model land in the correct passes independently.
class StaticMeshDrawCommand : public IDrawCommand
{

public:

	~StaticMeshDrawCommand()
	{
		//Logger::Log("a");
	}

	bgfx::VertexBufferHandle VertexBuffer = BGFX_INVALID_HANDLE;
	bgfx::IndexBufferHandle  IndexBuffer  = BGFX_INVALID_HANDLE;

	// Per-instance (not per-submesh) forward-pass data, computed in StaticMesh::FinalizeFrameData() and copied into every submesh command.
	vec3  LightAmbientColor = vec3(0);
	vec3  LightDirectColor  = vec3(0);
	vec3  LightDirection    = vec3(0, -1, 0);

	std::string PixelShader = "fs_default";

	bool DepthWrite  = true;
	bool DepthPrePass = true;
	bool TwoSided    = false;
	float Brightness = 1.2f;
	BgfxStateManager::Blend BlendMode = BgfxStateManager::Blend::Alpha;

	// Produced by StaticMesh::CollectCustomShaderParams() in PreFinalize (plain data - no callback into the mesh).
	std::map<std::string, vec4> CustomShaderParams;

	// Set when this submesh's name is on the owner's hide list; StaticMesh::CollectDrawCommands() skips these.
	bool Hidden = false;

	StaticMeshColorEmissiveMaterial Material;

	// Copied from roj::SkinnedMesh::shadowVolumePrecomp - non-owning handles. Left invalid unless CastDetailShadows is set, which makes DrawShadowVolumeStencil() a no-op.
	bgfx::VertexBufferHandle ShadowCapVbh    = BGFX_INVALID_HANDLE;
	bgfx::IndexBufferHandle  ShadowCapIbh    = BGFX_INVALID_HANDLE;
	uint32_t                 ShadowCapIndexCount = 0;
	bgfx::VertexBufferHandle ShadowEdgeVbh   = BGFX_INVALID_HANDLE;
	bgfx::IndexBufferHandle  ShadowEdgeIbh   = BGFX_INVALID_HANDLE;
	uint32_t                 ShadowEdgeIndexCount = 0;

	vec3 ShadowColorMultValue = vec3(1.0f);
	vec3 ShadowLightDirection = vec3(0, -1, 0);

	SurfaceType GetSurfaceType() const override { return Material.EffectiveSurfaceType(); }

	// Clusters by pixel-shader variant, the thing that actually changes bgfx program state.
	uint64_t GetSortKey() const override
	{
		return reinterpret_cast<uint64_t>(Material.GetForwardShader(PixelShader));
	}

	// Base loops CustomShaderParams. SkeletalMeshDrawCommand overrides, calls base, then adds bone matrices.
	virtual void ApplyAdditionalShaderParams(Shader* shader);

	void DrawForward(const mat4& view, const mat4& projection) override;
	void DrawDepth(const mat4& view, const mat4& projection) override;
	void DrawCustomId(const mat4& view, const mat4& projection) override;

	// Reserved for directional-light shadow-map cascades - currently unreachable (see IDrawCommand::DrawShadow).
	void DrawShadow(const mat4& view, const mat4& projection) override;

	void DrawShadowVolumeStencil(const mat4& view, const mat4& projection) override;
	vec3 GetShadowColorMult() const override { return ShadowColorMultValue; }

};

// SkeletalMesh points BoneMatrices at its own finalizedBoneTransforms once at load time - only the contents change per frame. This is a read-only view into data FinalizeFrameData() already computed, same category as a bgfx handle, not a callback into live mesh state.
class SkeletalMeshDrawCommand : public StaticMeshDrawCommand
{

public:

	const std::vector<mat4>* BoneMatrices = nullptr;

	void ApplyAdditionalShaderParams(Shader* shader) override;

};
