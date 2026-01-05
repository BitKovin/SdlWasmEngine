#pragma once

#include "../../StaticMesh.h"
#include "../../Renderer/Abstractions/RenderTexture.h"
#include "../UiContentBox.hpp"

class UiBilboard : public StaticMesh
{
public:
	UiBilboard(Entity* owner) : StaticMesh(owner) 
	{

		LoadFromFile("GameData/models/engine/widgetPlane.glb");

		DepthPrePath = false;

		PixelShader = "unlit_pixel";

		Transparent = true;

	}
	~UiBilboard();

	void DrawForward(mat4x4 view, mat4x4 projection) override;

	void FinalizeFrameData() override;

	UiContentBox ContentBox;
	ivec2 ViewportSize = ivec2(512, 512);

private:

	void DrawUi();
	void EnsureRenderTarget();

	RenderTexture* renderTexture = nullptr;

};

