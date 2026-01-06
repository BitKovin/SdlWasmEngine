#pragma once

#include "../../StaticMesh.h"
#include "../../Renderer/Abstractions/RenderTexture.h"
#include "../UiContentBox.hpp"
#include "../UiCanvas.hpp"

class UiBilboard : public StaticMesh
{
public:
	UiBilboard(Entity* owner) : StaticMesh(owner) 
	{

		LoadFromFile("GameData/models/engine/widgetPlane.glb");

		DepthPrePath = false;

		PixelShader = "unlit_pixel";

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

private:

	void DrawUi();
	void EnsureRenderTarget();

	RenderTexture* renderTexture = nullptr;

};

