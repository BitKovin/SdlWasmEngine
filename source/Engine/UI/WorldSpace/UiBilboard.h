#pragma once

#include "../../StaticMesh.h"
#include "../../Renderer/Abstractions/RenderTexture.h"
#include "../UiContentBox.hpp"
#include "../UiCanvas.hpp"

#include <vector>
#include <memory>

// ---------------------------------------------------------------------------
// NEW: Atlas system to solve framebuffer exhaustion in bgfx
// ---------------------------------------------------------------------------
struct BillboardAllocation
{
	class UiBilboardAtlas* atlas = nullptr;
	ivec4 rect = { 0, 0, 0, 0 };   // x, y, width, height in atlas pixels
	vec4 uvRect = { 0.0f, 0.0f, 1.0f, 1.0f }; // minU, minV, maxU, maxV
	bool IsValid() const { return atlas != nullptr && rect.z > 0 && rect.w > 0; }
};

class UiBilboardAtlas
{
public:
	UiBilboardAtlas(ivec2 size);
	~UiBilboardAtlas();

	bool TryAllocate(ivec2 reqSize, BillboardAllocation& outAlloc);
	void Free(const BillboardAllocation& alloc);

	RenderTexture* GetTexture() const { return texture; }

private:
	struct FreeRect { int x, y, w, h; };

	RenderTexture* texture = nullptr;
	std::vector<FreeRect> freeRects;
};

class UiBillboardAtlasManager
{
public:
	static UiBillboardAtlasManager& Get();

	BillboardAllocation Allocate(ivec2 size);
	void Free(BillboardAllocation& alloc);

private:
	std::vector<std::unique_ptr<UiBilboardAtlas>> atlases;
	const ivec2 kAtlasSize = { 4096, 4096 }; // safe for bgfx, plenty of room for UI billboards
};

class UiBilboard : public StaticMesh
{
public:
	UiBilboard(Entity* owner) : StaticMesh(owner)
	{
		LoadFromFile("GameData/models/engine/widgetPlane.glb");

		DepthPrePath = false;

		PixelShader = "fs_unlit_rect";

		Transparent = true;

		ContentBox = std::make_shared<UiContentBox>();
		ContentBox->pivot = vec2(0.5f, 0.5f);
		ContentBox->origin = vec2(0.5f, 0.5f);
		Canvas.AddChild(ContentBox);
	}
	~UiBilboard();

	void DrawForward(mat4x4 view, mat4x4 projection) override;

	void FinalizeFrameData() override;

	void Update();

	void PreDraw();

	std::shared_ptr<UiContentBox> ContentBox;
	UiCanvas Canvas;
	ivec2 ViewportSize = ivec2(512, 512);
	float PixelPerMeter = 100.0f;

private:
	void DrawUi();
	void EnsureAtlasAllocation();

	BillboardAllocation allocation;
};