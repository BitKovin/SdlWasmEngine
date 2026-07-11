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
public:
    string VideoPath = "GameData/cat.png";

    std::unique_ptr<VideoInstance> videoInstance = nullptr;

    UiVideo() {}
    ~UiVideo() {}

    void Update() override
    {
        UiElement::Update();
        if (videoInstance)
            videoInstance->Update(Time::DeltaTimeNoTimeScale);
    }

    void Draw() override
    {
        if (videoInstance == nullptr)
        {
            videoInstance = make_unique<VideoInstance>(AssetRegistry::GetVideoFromFile(VideoPath));
            videoInstance->Start();
            videoInstance->Update(0);
        }

        const std::vector<uint8_t>& frame = videoInstance->GetCurrentFrameData();
        int w = videoInstance->GetWidth();
        int h = videoInstance->GetHeight();

        if (!frame.empty() && w > 0 && h > 0)
            tex = std::make_unique<Texture>(frame.data(), w, h, bgfx::TextureFormat::RGB8, false);

        if (tex)
            DrawSelfTextured(tex->getHandle(), GetFinalColor(), static_cast<float>(tex->width), static_cast<float>(tex->height));

        UiElement::Draw();
    }

private:
    std::unique_ptr<Texture> tex;
};
