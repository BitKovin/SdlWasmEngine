#pragma once

#include "UiElement.h"

#include "UiRenderer.h"

#include "../Texture.hpp"
#include "../AssetRegistry.h"

#include "../Level.hpp"

#include "../Video/VideoInstance.h"

#include "../Time.hpp"

class UiVideo : public UiElement
{

private:

public:

	string VideoPath = "GameData/cat.png";

	std::unique_ptr<VideoInstance> videoInstance = nullptr;

	UiVideo()
	{

	}
	~UiVideo()
	{

	}

	void Update()
	{
		UiElement::Update();

		if (videoInstance)
		{
			videoInstance->Update(Time::DeltaTimeNoTimeScale);
		}
		
	}

	void Draw()
	{

		vec2 pos = finalizedPosition + finalizedOffset;

		if (videoInstance == nullptr)
		{
			videoInstance = make_unique<VideoInstance>(AssetRegistry::GetVideoFromFile(VideoPath));
			videoInstance->Start();
			videoInstance->Update(0);
		}

		// Get current RGB frame from video instance
		const std::vector<uint8_t>& frame = videoInstance->GetCurrentFrameData();
		int w = videoInstance->GetWidth();
		int h = videoInstance->GetHeight();

		// Make sure we have a valid frame
		if (!frame.empty() && w > 0 && h > 0) {
			// RGB format (3 bytes per pixel)
			tex = std::make_unique<Texture>(frame.data(), w, h, bgfx::TextureFormat::RGB8, false);
		}

		if (tex)
		{
			if (PixelShader.empty())
			{
				UiRenderer::DrawTexturedRect(pos, finalizedSize, rotation, pivot, tex->getHandle(), GetFinalColor());
			}
			else
			{
				UiRenderer::DrawTexturedRectShader(pos, finalizedSize, rotation, pivot, tex->getHandle(), GetFinalColor(), PixelShader);
			}
		}


		UiElement::Draw();
	}

private:

	std::unique_ptr<Texture> tex;

};