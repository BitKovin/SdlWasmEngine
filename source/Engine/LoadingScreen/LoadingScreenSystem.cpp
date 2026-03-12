#include "LoadingScreenSystem.h"
#include "../UI/UiRenderer.h"
#include "../EngineMain.h"
#include "../AssetRegistry.h"



void LoadingScreenSystem::Init()
{

}

void LoadingScreenSystem::SetLoadingCanvas(std::shared_ptr<UiLoadingScreenBase> canvas)
{

	viewport.ClearChildren();

	viewport.AddChild(canvas);
	uiCanvas = canvas;

}

void LoadingScreenSystem::Update(float newProgress)
{

	Progress = newProgress;
	Draw();

}

void LoadingScreenSystem::Draw()
{
    if (uiCanvas == nullptr)
        return;

    uiCanvas->LoadingProgress = Progress;

    const uint16_t viewId = 0; // loading screen view

    const uint16_t width = EngineMain::MainInstance->ScreenSize.x;
    const uint16_t height = EngineMain::MainInstance->ScreenSize.y;

    // Configure view
    bgfx::setViewRect(viewId, 0, 0, width, height);
    bgfx::setViewClear(
        viewId,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x000000ff, // black
        1.0f,
        0
    );

    bgfx::touch(viewId); // ensure view clears even if nothing submitted

    // Update and draw UI
    viewport.Update();
    viewport.FinalizeChildren();
    viewport.Draw(); // must submit bgfx draw calls internally

    // Process events (still needed for SDL)
    SDL_PollEvent(nullptr);

    // Present frame
    bgfx::frame();
}

