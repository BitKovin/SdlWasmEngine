#pragma once

#include "../DrawCommands/IDrawCommand.h"
#include "../VertexData.h"

#include <bgfx/bgfx.h>
#include <BgfxStateManager.h>
#include <vector>
#include <string>

class Texture;
struct InstanceData;

// All three are fully self-contained - fields committed by value in the owning emitter's FinalizeFrameData(), Draw*() never touches the emitter/Entity.
class ParticleDrawCommand : public IDrawCommand
{

public:

	std::vector<InstanceData> Instances;

	Texture* ResolvedTexture = nullptr;
	bool IsDecal = false;
	std::string PixelShader = "fs_unlit";
	BgfxStateManager::Blend BlendMode = BgfxStateManager::Blend::Alpha;

	SurfaceType GetSurfaceType() const override { return SurfaceType::Transparent; }

	void DrawForward(const mat4& view, const mat4& projection) override;

};

// Ribbon/TrailEmitter build Vertices/Indices in PreFinalize(); DrawForward just allocates transient buffers and submits.
class RibbonDrawCommand : public IDrawCommand
{

public:

	std::vector<VertexData> Vertices;
	std::vector<uint32_t> Indices;

	Texture* ResolvedTexture = nullptr;
	std::string PixelShader = "fs_unlit";
	BgfxStateManager::Blend BlendMode = BgfxStateManager::Blend::Alpha;

	SurfaceType GetSurfaceType() const override { return SurfaceType::Transparent; }

	void DrawForward(const mat4& view, const mat4& projection) override;

};

class TrailDrawCommand : public IDrawCommand
{

public:

	std::vector<VertexData> Vertices;
	std::vector<uint32_t> Indices;

	Texture* ResolvedTexture = nullptr;
	std::string PixelShader = "fs_default";
	BgfxStateManager::Blend BlendMode = BgfxStateManager::Blend::Alpha;

	SurfaceType GetSurfaceType() const override { return SurfaceType::Transparent; }

	void DrawForward(const mat4& view, const mat4& projection) override;

};
