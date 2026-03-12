#include "UiBilboard.h"

#include "../UiRenderer.h"

#include <BgfxStateManager.h>
#include <Renderer/Abstractions/ViewIdManager.h>


static std::map<hashed_string, std::vector<RenderTexture*>> bilboardRtCache;

UiBilboard::~UiBilboard()
{
	if (renderTexture)
	{
		hashed_string hs = to_string(renderTexture->width()) + ";" + to_string(renderTexture->height());
		bilboardRtCache[hs].push_back(renderTexture);
		renderTexture = nullptr;
	}
}

void UiBilboard::DrawForward(mat4x4 view, mat4x4 projection)
{
	if (renderTexture == nullptr) return;

	// Point the mesh's colour slot at the UI render target so StaticMesh picks
	// it up through the normal ColorTextureId path (same as any other override).
	ColorTextureId = (int)renderTexture->textureHandle().idx;

	StaticMesh::DrawForward(view, projection);
}

void UiBilboard::FinalizeFrameData()
{
	Rotation = Camera::rotation;

	Canvas.size = vec2((float)ViewportSize.x, (float)ViewportSize.y);

	Scale.x = (float)ViewportSize.x / PixelPerMeter;
	Scale.y = (float)ViewportSize.y / PixelPerMeter;

	Canvas.FinalizeChildren();

	StaticMesh::FinalizeFrameData();
}

void UiBilboard::Update()
{
	Canvas.Update();
	Canvas.UpdateChildrenOffsetRecursive();
}

void UiBilboard::DrawUi()
{
	EnsureRenderTarget();

	UiRenderer::customViewport = true;
	UiRenderer::customViewportSize = ViewportSize;

	// --- bind the billboard's own render target ---
	renderTexture->setAsRenderTarget();

	bgfx::setViewRect(
		ViewIdManager::GetCurrentId(),
		0, 0,
		(uint16_t)ViewportSize.x,
		(uint16_t)ViewportSize.y
	);

	// Clear to transparent black, matching the main UI pass in EngineMain
	bgfx::setViewClear(
		ViewIdManager::GetCurrentId(),
		BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
		0x00000000,
		1.0f, 0
	);

	auto savedState = BgfxStateManager::GetState();

	BgfxStateManager::Reset();
	BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
	// Premultiplied-alpha: replaces glBlendFuncSeparate(ONE, ONE_MINUS_SRC_ALPHA, ...)
	BgfxStateManager::SetBlend(BgfxStateManager::Blend::Premultiplied);

	Canvas.Draw();

	UiRenderer::EndFrame();

	BgfxStateManager::SetState(savedState);

	UiRenderer::customViewport = false;
}

void UiBilboard::PreDraw()
{
	StaticMesh::PreDraw();

	DrawUi();
}

void UiBilboard::EnsureRenderTarget()
{
	hashed_string hs = to_string(ViewportSize.x) + ";" + to_string(ViewportSize.y);

	// Try to reuse from cache first
	auto& vec = bilboardRtCache[hs];
	if (!vec.empty())
	{
		if (renderTexture)
		{
			// Return the old one to cache before swapping
			hashed_string oldHS = to_string(renderTexture->width()) + ";" + to_string(renderTexture->height());
			bilboardRtCache[oldHS].push_back(renderTexture);
		}

		renderTexture = vec.back();
		vec.pop_back();
		return;
	}

	// No cached RT found: create new or resize existing
	if (renderTexture == nullptr)
	{
		renderTexture = new RenderTexture(
			ViewportSize.x,
			ViewportSize.y,
			TextureFormat::RGBA8,
			TextureType::Texture2D,
			false,
			BGFX_SAMPLER_U_CLAMP
			| BGFX_SAMPLER_V_CLAMP
			| BGFX_SAMPLER_MIN_POINT
			| BGFX_SAMPLER_MAG_POINT
		);

		renderTexture->SetName(
			"UiBilboard RT " + to_string(ViewportSize.x) + "x" + to_string(ViewportSize.y)
		);
	}
	else if (renderTexture->width() != (uint32_t)ViewportSize.x ||
		renderTexture->height() != (uint32_t)ViewportSize.y)
	{
		renderTexture->resize(ViewportSize.x, ViewportSize.y);
	}
}