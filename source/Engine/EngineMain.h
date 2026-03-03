#pragma once

#include "malloc_override.h"

#include <SDL2/SDL_video.h>

#include "Camera.h"
#include "Input.h"

#include "Level.hpp"

#include "ShaderManager.h"

#include "ThreadPool.h"

#include <future>
#include <thread>

#include "ImGuiEngineImpl.h"

#include "UI/UiViewport.hpp"

#include "MapParser.h"

#include "Renderer/Renderer.h"
#include "SaveSystem/LevelSaveSystem.h"

class RmlUiContext;

class EngineMain
{
private:



public:

    Renderer* MainRenderer;

	SDL_Window* Window = nullptr;

	static EngineMain* MainInstance;

    static UiViewport Viewport;

    ivec2 ScreenSize = ivec2();

    bool Paused = false;

    bool DebugUiEnabled = false;

    int LoadingFrames = 0;

    unsigned long frame = 0;

	bool SimulatingGameTicks = false;
    bool SimulatingPreciseGameTicks = false;

	RenderTexture* UiRenderTexture = nullptr;

	RmlUiContext* RmlContext = nullptr;

	EngineMain(SDL_Window* window)
	{
		Window = window;
	}
	~EngineMain()
	{
		FileSystemEngine::Shutdown();
        Level::Current->CloseLevel();
	}


    std::map<std::string, std::vector<std::string>> Arguments;

    void UpdateScreenSize();

    ThreadPool* MainThreadPool;

    void ToggleFullscreen();

    void initGame();

    void InitInputs();



    void Init(const std::vector<std::string> args = {});

    static std::map<std::string, std::vector<std::string>> ParseCommands(const std::vector<std::string>& args);

    void FinishFrame();

    // Toggle asynchronous GameUpdate.
    bool asyncGameUpdate = true;

    // Store the future of the async update.
    std::future<void> gameUpdateFuture;

    // Main game loop.
    void MainLoop();

	void SimulateGameTick();
	void SimulateGameTicksForTime(float timeToSimulate);
    void SimulateGameTicksForTimeCombinedPrecision(float timeToSimulate);

    void GameUpdate();

    void Render();

	void ForceUpdateScreenSize();

    void FinishRender();

};