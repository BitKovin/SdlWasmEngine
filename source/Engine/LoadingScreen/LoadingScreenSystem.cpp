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

	if (uiCanvas == nullptr) return;


	Framebuffer::unbind();

	uiCanvas->LoadingProgress = Progress;

	glViewport(0, 0, EngineMain::MainInstance->ScreenSize.x, EngineMain::MainInstance->ScreenSize.y);

	glClearColor(0, 0, 0, 1);
	glDisable(GL_DEPTH_TEST);

	viewport.Update();
	viewport.FinalizeChildren();
	viewport.Draw();

	glFinish();
	glFlush();
	SDL_PollEvent(nullptr);
	SDL_GL_SwapWindow(EngineMain::MainInstance->Window);

}

