#include "UiBilboard.h"

#include "../UiRenderer.h"

UiBilboard::~UiBilboard()
{
	if (renderTexture)
	{
		delete renderTexture;
		renderTexture = nullptr;
	}
}

void UiBilboard::DrawForward(mat4x4 view, mat4x4 projection)
{

	ColorTextureId = renderTexture->id();

	StaticMesh::DrawForward(view, projection);

}

void UiBilboard::FinalizeFrameData()
{

	Rotation = Camera::rotation;

	Canvas.size = vec2((float)ViewportSize.x, (float)ViewportSize.y);

	Canvas.FinalizeChildren();

	StaticMesh::FinalizeFrameData();
}

void UiBilboard::Update()
{
	Canvas.Update();
}

void UiBilboard::DrawUi()
{

	EnsureRenderTarget();

	UiRenderer::customViewport = true;

	UiRenderer::customViewportSize = ViewportSize;

	renderTexture->bindFramebuffer();
	glViewport(0, 0, ViewportSize.x, ViewportSize.y);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); // alpha = 0
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Canvas.Draw();

	UiRenderer::customViewport = false;

}

void UiBilboard::PreDraw()
{
	StaticMesh::PreDraw();

	DrawUi();

}

void UiBilboard::EnsureRenderTarget()
{

	if (renderTexture == nullptr)
	{
		renderTexture = new RenderTexture(ViewportSize.x, ViewportSize.y, TextureFormat::RGBA8, TextureType::Texture2D, false, GL_LINEAR, GL_LINEAR, GL_CLAMP_TO_EDGE);

	}
	else if (renderTexture->width() != ViewportSize.x || renderTexture->height() != ViewportSize.y)
	{
		renderTexture->resize(ViewportSize.x, ViewportSize.y);
	}

}
