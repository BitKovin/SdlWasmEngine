#include "EngineMain.h"

#include <LevelObjectFactory.h>
#include <Time.hpp>
#include <Physics.h>
#include <SoundSystem/SoundManager.hpp>
#include <Particle/ParticleEmitter.h>
#include <DebugDraw.hpp>
#include <UI/UiRenderer.h>
#include <LoadingScreen/LoadingScreenSystem.h>
#include <FileSystem/FileSystem.h>
#include <BehaviourTree/BehaviorTree.h>
#include <AiPerception/AiPerceptionSystem.h>
#include <Console/Console.h>
#include <Console/ConsoleDefaultCommands.h>
#include <UI/RmlUi/RmlUiContext.h>
#include <LevelTraversalSystem.h>
#include <AssetRegistry.h>

#include <BgfxStateManager.h>
#include <Renderer/Abstractions/ViewIdManager.h>
#include <BgfxResetManager.h>

#include <FileSystem/NativeFileSystem.h>
#include <FileSystem/ZipVFS.h>

#include <memory>

#include <UI/UiNavigation.h>
#include <UI/UiFocusPointer.h>

#include <Profiling/ResourceStatistics.hpp>

#include <Analytics/AnalyticsSystem.h>

#include <Logger.hpp>

#include <tracy/tracy/Tracy.hpp>

#include <Network/NetworkManager.h>

#include <Localization/Localisation.h>

#include <Ecs/Ecs.h>


EngineMain* EngineMain::MainInstance = nullptr;

UiViewport EngineMain::Viewport;

#if __EMSCRIPTEN__

#include "emscripten/emscripten.h"
#include "emscripten/html5.h"
#include "FileSystem/FileSystem.h"

EM_JS(int, canvas_get_width, (), {
return canvas.width;
    });
EM_JS(int, canvas_get_height, (), {
  return canvas.height;
    });

#endif // __EMSCRIPTEN__

EngineMain::~EngineMain()
{
    GameUpdateSingleThreadPool->Stop();
    AssetRegistry::StopLoaderThread();

    NetworkManager::Shutdown();
    FileSystemEngine::Shutdown();
    Level::Current->CloseLevel();
	UiRenderer::Shutdown();
    ParticleEmitter::DestroyBillboardVao();
    delete(RmlUiContext::Main);
    RmlUiContext::Main = nullptr;
    ShaderManager::Shutdown();

    delete GameUpdateSingleThreadPool;
    GameUpdateSingleThreadPool = nullptr;
}

void EngineMain::UpdateScreenSize()
{
    glm::ivec2 oldScreenSize = ScreenSize;

#if __EMSCRIPTEN__
    int width, height;
    EMSCRIPTEN_RESULT result = emscripten_get_canvas_element_size("#canvas", &width, &height);
    if (result == EMSCRIPTEN_RESULT_SUCCESS)
    {
        ScreenSize = ivec2(width, height);
    }
    else
    {
        Logger::Warning("failed to get screen resolution");
    }
#else
    int w, h;
    SDL_GetWindowSize(Window, &w, &h);
    ScreenSize.x = w;
    ScreenSize.y = h;
#endif

    if (ScreenSize != oldScreenSize)
    {
        if (RmlContext)
            RmlContext->OnResize(ScreenSize.x, ScreenSize.y);

        BgfxResetManager::SetResolution(ScreenSize);
    }
}

void EngineMain::ToggleFullscreen()
{


    Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
    bool IsFullscreen = SDL_GetWindowFlags(Window) & FullscreenFlag;



    // Toggle fullscreen
    SDL_SetWindowFullscreen(Window, IsFullscreen ? 0 : FullscreenFlag);



    SDL_ShowCursor(SDL_ENABLE);

}

void EngineMain::initGame()
{
    Entity* ent = LevelObjectFactory::instance().create("gamestart");

    if (ent == nullptr)
    {
        Logger::Log("failed to create gamestart entity");
        return;
    }
    Level::Current = new Level();
    Level::Current->AddEntity(ent);
    ent->Start();

    //ShaderManager::CompilePSOsFromFile("GameData/PSOs/pso_cache.json");

    return;

    Level::OpenLevel("GameData/Maps/test2.map");


    for (size_t i = 0; i < 000; i++) 
    {
        float angle = i * 0.1f; // Angle step controls tightness of the spiral
        float radius = 1 + 0.05f * i; // Radius increases over time
        float x = radius * cos(angle) + 10;
        float y = 50;
        float z = radius * sin(angle);

        Entity* ent = LevelObjectFactory::instance().create("testnpc");

        ent->Position = vec3(x, y, z);
        ent->Start();

        Level::Current->AddEntity(ent);
    }

}

void EngineMain::InitInputs()
{

    Input::AddAction("click")->AddMouseButton(0);


}

void EngineMain::Init(std::vector<std::string> args)
{

#ifdef DISTRIBUTION
    DebugUiEnabled = false;
#endif // DISTRIBUTION

    BgfxResetManager::Reset();

    tracy::SetThreadName("Main Thread");

    Arguments = ParseCommands(args);

    if (FileSystem == nullptr)
        FileSystem = std::make_shared<NativeFileSystem>();

	FileSystemEngine::AddFileSystem(FileSystem);
    FileSystemEngine::AddFileSystem(std::make_shared<ZipVFS>(FileSystem.get()));

    LevelObjectFactory::instance().registerDefaults();
    BehaviorTree::RegisterTypes();

	Localisation::Initialize("GameData/localisation", "en");

    UpdateScreenSize();

    EcsScheduler::Finalize();

    Logger::Info("init");

    MainThreadPool = new ThreadPool("Thread Pool");

    MainThreadPool->Start(ThreadPool::GetNumThreadsForThreadPool());

	GameUpdateSingleThreadPool = new ThreadPool("Game Thread Pool");
	GameUpdateSingleThreadPool->Start(1);

	AssetRegistry::StartLoaderThread();

    SoundManager::Initialize();

    Time::Init();

    ConsoleDefaultCommands::RegisterAll();

    Logger::Info("initPhys");
    Physics::Init();
    Logger::Info("initPhys");

    FileSystemEngine::Init();

    MainRenderer = new Renderer();

    UiRenderer::Init();

	RmlContext = new RmlUiContext(Window, 800, 600, true);
	RmlContext->Initialize();
	RmlUiContext::Main = RmlContext;

    ParticleEmitter::InitBilboardVaoIfNeeded();

    InitInputs();

    BgfxResetManager::SetMaxAnisotropy(true);

    Level::Current = new Level();

    bgfx::touch(0);
	bgfx::frame();


    ImStartFrame();
	RenderImGui();
    bgfx::touch(0);
    bgfx::frame();

    AnalyticsSystem::Set(new AnalyticsSystem());

}

#include <map>
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>

std::map<std::string, std::vector<std::string>> EngineMain::ParseCommands(const std::vector<std::string>& args)
{
    std::map<std::string, std::vector<std::string>> out;
    bool positionalOnly = false;

    auto isNegativeNumber = [](const std::string& s) {
        if (s.size() < 2 || s[0] != '-') return false;
        if (std::isdigit((unsigned char)s[1])) return true;
        if (s[1] == '.' && s.size() > 2 && std::isdigit((unsigned char)s[2])) return true;
        return false;
        };

    auto isOptionToken = [&](const std::string& s) {
        if (s.empty() || s == "-") return false;
        if (s == "--") return true;
        if (s[0] != '-') return false;
        if (isNegativeNumber(s)) return false;
        return true;
        };

    auto stripDashes = [](const std::string& s) {
        size_t i = 0;
        while (i < s.size() && s[i] == '-') ++i;
        return s.substr(i);
        };

    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& tok = args[i];

        if (!positionalOnly && tok == "--") {
            positionalOnly = true;
            continue;
        }

        if (!positionalOnly && isOptionToken(tok)) {
            std::string body = stripDashes(tok);

            // --key=value or -key=value
            size_t eq = body.find('=');
            if (eq != std::string::npos) {
                std::string k = body.substr(0, eq);
                std::string v = body.substr(eq + 1);
                out[k].push_back(v);
                continue;
            }

            bool singleDash = (tok.size() >= 1 && tok[0] == '-' && !(tok.size() > 1 && tok[1] == '-'));

            // Single-dash multi-character token
            if (singleDash && body.size() > 1) {
                const std::string* nextTok = (i + 1 < args.size()) ? &args[i + 1] : nullptr;
                bool nextIsValue = nextTok && !isOptionToken(*nextTok);

                if (nextIsValue) {
                    // Treat as long option with value
                    out[body].push_back(*nextTok);
                    ++i; // skip next token
                    continue;
                }
                else {
                    // Treat as grouped short flags if all letters
                    bool allLetters = std::all_of(body.begin(), body.end(), [](char c) { return std::isalpha((unsigned char)c); });
                    if (allLetters) {
                        for (char c : body) {
                            std::string k(1, c);
                            out[k]; // presence
                        }
                        continue;
                    }
                    else {
                        // fallback: treat as long option without value
                        out[body];
                        continue;
                    }
                }
            }

            // Single-letter flag: -a
            if (body.size() == 1) {
                out[body];
                continue;
            }

            // Double-dash long option without value
            out[body];
        }
        else {
            // Positional or value for previous option
            if (!out.empty()) {
                // find last key without value
                auto it = std::find_if(out.rbegin(), out.rend(), [](const auto& p) { return p.second.empty(); });
                if (it != out.rend()) {
                    const std::string& lastKey = it->first;
                    out[lastKey].push_back(tok);
                    continue;
                }
            }
            // positional argument
            out["_"].push_back(tok);
        }
    }

    return out;
}


void EngineMain::FinishFrame()
{
    AssetRegistry::AdvanceFrame(); // drives AssetLoadState::lastUsedFrame

    Level::Current->RemovePendingEntities();
    Level::Current->MemoryCleanPendingEntities();

    Camera::Update(Time::DeltaTime);
    Level::Current->FinalizeFrame();
    

    NavigationSystem::DrawNavmesh();

    DebugDraw::Finalize();

    UpdateScreenSize();

    Camera::ScreenHeight = ScreenSize.y;

    float AspectRatio = static_cast<float>(ScreenSize.x) / static_cast<float>(ScreenSize.y);
    Camera::AspectRatio = AspectRatio;

}

void EngineMain::MainLoop()
{
    
    ZoneScopedN("Frame");
    FrameMark;

    BgfxResetManager::ApplyIfNeeded();




    if (DebugUiEnabled)
    {
        ImStartFrame();
    }
    

    bool loadedlevel = Level::LoadPendingLevel();

    if (loadedlevel)
    {
        auto pointer = std::make_shared<UiFocusPointer>();
        pointer->ImagePath = "GameData/cat.png";
        pointer->PointerSize = vec2(24.f, 24.f);
        pointer->PointerOffset = 10.f;
        Viewport.AddChild(pointer);
    }

    if (frame == 5) //some platforms require it
    {

        ForceUpdateScreenSize(); //hack to fix some rendering issues that happen on some platforms
        initGame();

        if (pendingRestoreSaveData.LevelData.name.empty() == false)//android restore takes priority of level loading
        {
            GameSaveSystem::LoadGameFromData(pendingRestoreSaveData);
        }

    }

    Viewport.ResetTouchInputs();

    for (auto& event : Input::TouchActions)
    {

        auto hit = Viewport.GetHitElementUnderPosition(event.second.position);

        if (hit == nullptr) continue;

        hit->TouchEvents.push_back(event.second);

    }
    Viewport.TouchInputPostProcessing();


    Level::Current->LoadAssets();

	bool simulatedGameTicks = false;

    if (Input::GetAction("dbg_simulate")->Pressed())
    {
		SimulateGameTicksForTime(1000.0f);

        simulatedGameTicks = true;

    }

    if (loadedlevel || simulatedGameTicks)
    {

        LoadingFrames = 0;

        if (loadedlevel)
        {
            if (LevelTraversalSystem::TimeSkip > 0)
            {
                SimulateGameTicksForTime(LevelTraversalSystem::TimeSkip);
            }
        }

        Time::Update();
        Time::DeltaTime = 0.05;
        Time::DeltaTimeF = 0.05f;

        LoadingFrames = 5;

    }
    LevelTraversalSystem::TimeSkip = 0;

    Input::Update();

    UiNavigation::Update(&Viewport);

    Viewport.Update();

    UiNavigation::LateUpdate();

    Viewport.UpdateChildrenOffsetRecursive();
    Viewport.FinalizeChildren();

    Input::UpdateMouse();

    
    if (asyncGameUpdate)
    {
        ZoneScopedN("GameUpdate (Async)");

        GameUpdateSingleThreadPool->QueueJob([this]()
            {
                ZoneScopedN("GameUpdate Worker Job");
                tracy::SetThreadName("Game Thread");
                GameUpdate();
            });
    }
    else
    {
        ZoneScopedN("GameUpdate (Sync)");
        GameUpdate();

        FinishFrame();

    }

    Render();

    FinishRender();

    if (asyncGameUpdate)
    {
        ZoneScopedN("Wait GameUpdate Jobs");
        GameUpdateSingleThreadPool->WaitForFinish();

        FinishFrame();
    }


	RmlContext->Update(Time::DeltaTimeFNoTimeScale);


    Time::Update();

    if (Input::GetAction("fullscreen")->Pressed())
    {
        ToggleFullscreen();

        //Level::OpenLevel("GameData/Maps/test.map");

    }

    if (LoadingFrames == 2)
    {
        AssetRegistry::EndLevelLoad();
    }

    frame++;

}

void EngineMain::SimulateGameTick()
{

	float fixedDeltaTime = 1.0f / 10.0f;

	Time::DeltaTimeF = fixedDeltaTime;
	Time::DeltaTime = (double)fixedDeltaTime;
	Time::DeltaTimeFNoTimeScale = fixedDeltaTime;
	Time::GameTime += fixedDeltaTime;
	Time::GameTimeNoPause += fixedDeltaTime;
    
	SimulatingGameTicks = true;

	GameUpdate();

    SimulatingGameTicks = false;

}

void EngineMain::SimulateGameTicksForTime(float timeToSimulate)
{

    Input::ReleaseAllActions();

    LoadingScreenSystem::Progress = 0.0f;
    LoadingScreenSystem::Draw();

    Logger::Log("Simulating Seconds: " + to_string(timeToSimulate));

    const float lowDt = 1.0f / 5.0f;
    const float highDt = 1.0f / 20.0f;
    const float highPrecisionTime = 10.0f;

    SimulatingGameTicks = true;

    Camera::position = vec3(0, -1000000, 0);
    Camera::rotation = MathHelper::FindLookAtRotation(vec3(), vec3(0, -100, 0));

	FinishFrame();

    // -----------------------------
    // Tick count calculation
    // -----------------------------
    float lowPrecisionTime =
        std::max(0.0f, timeToSimulate - highPrecisionTime);

    int lowPrecisionTicks = (int)(lowPrecisionTime / lowDt);
    int highPrecisionTicks = (int)(
        (timeToSimulate - lowPrecisionTicks * lowDt) / highDt
        );

    int totalTicks = lowPrecisionTicks + highPrecisionTicks;

    // -----------------------------
    // Low precision phase
    // -----------------------------
    Time::DeltaTimeF = lowDt;
    Time::DeltaTime = (double)lowDt;
    Time::DeltaTimeFNoTimeScale = lowDt;


    Entity* player = Level::Current->FindEntityWithName("player");

    vec3 oldPlayerPosition = vec3();

    vec3 farAwayPosition = vec3(100000, 1000000, 1000000);

    if (player)
    {

        oldPlayerPosition = player->Position;

        player->Teleport(farAwayPosition);

        player->Update();
        player->AsyncUpdate();
        player->LateUpdate();

    }


	int onePercentTicks = glm::max(1, totalTicks / 200);

    for (int i = 0; i < lowPrecisionTicks; i++)
    {
        Time::GameTime += lowDt;
        Time::GameTimeNoPause += lowDt;
        GameUpdate();
        Level::Current->RemovePendingEntities();
        Level::Current->MemoryCleanPendingEntities();
        frame++;
        if (i % onePercentTicks == 0)
        {
            LoadingScreenSystem::Progress =
                (float)i / (float)totalTicks;
            LoadingScreenSystem::Draw();
        }
    }

    // -----------------------------
    // High precision phase (last 10s)
    // -----------------------------
    Time::DeltaTimeF = highDt;
    Time::DeltaTime = (double)highDt;
    Time::DeltaTimeFNoTimeScale = highDt;
    SimulatingPreciseGameTicks = true;
    for (int i = 0; i < highPrecisionTicks; i++)
    {
        Time::GameTime += highDt;
        Time::GameTimeNoPause += highDt;
        GameUpdate();
        Level::Current->RemovePendingEntities();
        Level::Current->MemoryCleanPendingEntities();

		frame++;

        if (i % 10 == 0)
        {
            LoadingScreenSystem::Progress =
                (float)(lowPrecisionTicks + i) / (float)totalTicks;
            LoadingScreenSystem::Draw();
        }
    }
    SimulatingPreciseGameTicks = false;
    if (player)
    {
        player->Teleport(oldPlayerPosition);

    }

    FinishFrame();

    SimulatingGameTicks = false;
    LoadingFrames = 5;
}


void EngineMain::SimulateGameTicksForTimeCombinedPrecision(float timeToSimulate)
{


    const float highPrecissionSimulationTime = 10;

    float timeToSimulateHighPrecission = glm::min(highPrecissionSimulationTime, timeToSimulate);

    float timeToSimulateLowPrecission = glm::clamp(timeToSimulate - timeToSimulateHighPrecission, 0.f, 1000000.f);

	SimulateGameTicksForTime(timeToSimulateLowPrecission);
	SimulateGameTicksForTime(timeToSimulateHighPrecission);

}


void EngineMain::GameUpdate()
{
    ZoneScopedN("GameUpdate");

    {
        ZoneScopedN("Physics");
        if (Paused == false)
            Physics::Simulate();
    }

    {
        ZoneScopedN("Physics Debug");
        Physics::UpdateDebugDraw();
    }

    {
        ZoneScopedN("Level Physics");
        Level::Current->UpdatePhysics();
    }

    {
        ZoneScopedN("AI Perception");
        AiPerceptionSystem::Update();
    }

    {
        ZoneScopedN("ECS PreUpdate");
        EcsScheduler::RunTickGroup(TickGroup::PreUpdate, EcsWorld::Registry(), Time::DeltaTimeF, Paused);
    }

    {
        ZoneScopedN("Level Update");
        Level::Current->Update(Paused);
    }

    {
        ZoneScopedN("ECS PostUpdate");
        EcsScheduler::RunTickGroup(TickGroup::PostUpdate, EcsWorld::Registry(), Time::DeltaTimeF, Paused);
    }

    {
        ZoneScopedN("Async Update");
        Level::Current->AsyncUpdate(Paused);
    }

    {
        ZoneScopedN("ECS PostAsyncUpdate");
        EcsScheduler::RunTickGroup(TickGroup::PostAsyncUpdate, EcsWorld::Registry(), Time::DeltaTimeF, Paused);
    }

    {
        ZoneScopedN("Late Update");
        Level::Current->LateUpdate(Paused);
    }

    {
        ZoneScopedN("ECS PostLateUpdate");
        EcsScheduler::RunTickGroup(TickGroup::PostLateUpdate, EcsWorld::Registry(), Time::DeltaTimeF, Paused);
    }

    {
        ZoneScopedN("AnalyticsSystem Update");
        AnalyticsSystem::Get().Tick(Time::GameTime);
    }

    {
        ZoneScopedN("NetworkManager Update");

        NetworkManager::Tick(Time::DeltaTimeF);
        Level::Current->AddPendingLevelObjects();
        Level::Current->RemovePendingEntities();

    }

    {
        ZoneScopedN("Pre Finalize");
        Level::Current->PreFinalize();
    }

    if (!SimulatingGameTicks)
    {
        ZoneScopedN("Audio");
        SoundManager::Update();
    }
}


void EngineMain::Render()
{

    ZoneScopedN("Render");

    // Budgeted, time-limited - see AssetRegistry.h. Has to run before
    // anything below touches bgfx, since this is the only place lazily
    // loaded textures/meshes actually get uploaded.
    AssetRegistry::ProcessPendingUploads(AssetRegistry::UploadBudgetMs);

    ivec2 uiResolution = ivec2(
        UiManager::GetScaledUiHeight() * Camera::AspectRatio,
        UiManager::GetScaledUiHeight()
    );

    /* ============================================================
       CREATE / RESIZE UI RENDER TARGET
       ============================================================ */

    if (UiFrameBuffer == nullptr)
    {
        UiFrameBuffer = new Framebuffer();
        UiRenderTexture = new RenderTexture(
            uiResolution.x,
            uiResolution.y,
            TextureFormat::RGBA8);
        UiRenderTextureStencil = new RenderTexture(uiResolution.x,
            uiResolution.y, TextureFormat::Depth24Stencil8);

        UiFrameBuffer->attachColor(UiRenderTexture);
        UiFrameBuffer->attachDepth(UiRenderTextureStencil);
    }

    if (UiRenderTexture->width() != (uint32_t)uiResolution.x ||
        UiRenderTexture->height() != (uint32_t)uiResolution.y)
    {
        UiRenderTexture->resize(uiResolution.x, uiResolution.y);
        UiRenderTextureStencil->resize(uiResolution.x, uiResolution.y);

        // Textures got new handles — rebuild the framebuffer against them
        UiFrameBuffer->attachColor(UiRenderTexture);        // ← FIX 1
        UiFrameBuffer->attachDepth(UiRenderTextureStencil);
    }

	if (FinalFrameRenderTexture == nullptr || FinalFrameRenderTexture->height() != (uint32_t)ScreenSize.y || FinalFrameRenderTexture->width() != (uint32_t)ScreenSize.x)
	{
		if (FinalFrameRenderTexture)
			delete FinalFrameRenderTexture;
		FinalFrameRenderTexture = new RenderTexture(
			ScreenSize.x,
			ScreenSize.y,
			TextureFormat::RGBA8);
	}

    /* ============================================================
       UI PASS → render to transparent RT
       ============================================================ */
    {

        ZoneScopedN("UI Pass");

        UiFrameBuffer->bind();
        bgfx::setViewMode(ViewIdManager::GetCurrentId(), bgfx::ViewMode::Sequential);
        bgfx::setViewClear(ViewIdManager::GetCurrentId(),
            BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL,
            0x00000000, // transparent black
            1.0f, 0);   // stencil cleared to 0

        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
        BgfxStateManager::SetWriteDepth(false);
        BgfxStateManager::SetBlend(BgfxStateManager::Blend::Premultiplied);

        bgfx::touch(ViewIdManager::GetCurrentId());
        {
            ZoneScopedN("Viewport Draw");
            Viewport.Draw();
            UiElement::DrawingLate = true;
        }
        {
            ZoneScopedN("Late UI");
            for (auto elem : UiElement::pendingLateDrawElements)
            {
                elem->Draw();
            }
        }

        UiElement::DrawingLate = false;
        UiElement::pendingLateDrawElements.clear();

        UiRenderer::EndFrame();

        UiFrameBuffer->unbind();
    }

    /* ============================================================
       WORLD PASS — RenderLevel manages its own view IDs internally
       ============================================================ */

    {
        ZoneScopedN("World Render");
        MainRenderer->RenderLevel(Level::Current, FinalFrameRenderTexture->frameBufferHandle());

        bgfx::setViewRect(ViewIdManager::GetCurrentId(), 0, 0,
            (uint16_t)ScreenSize.x,
            (uint16_t)ScreenSize.y);
        //bgfx::setViewFrameBuffer(ViewIdManager::GetCurrentId(), FinalFrameRenderTexture->frameBufferHandle());

        ViewIdManager::GiveNextId();

		bgfx::setViewFrameBuffer(ViewIdManager::GetCurrentId(), BGFX_INVALID_HANDLE);
		bgfx::setViewRect(ViewIdManager::GetCurrentId(), 0, 0,
			(uint16_t)ScreenSize.x,
			(uint16_t)ScreenSize.y);

        auto copyShader = MainRenderer->copyShader;

		copyShader->SetTexture("screenTexture", FinalFrameRenderTexture->textureHandle());
        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
        BgfxStateManager::Apply();
		MainRenderer->RenderFullscreenQuad(copyShader);
    }

    /* ============================================================
       COMPOSITE UI OVER SCENE → swapchain (view 0)
       ============================================================ */
    {

        ZoneScopedN("Composite UI Pass");



        auto compositeShader = ShaderManager::GetShaderProgram(
            "vs_fullscreen",
            "fs_fxaa_simple"
        );

        compositeShader->SetTexture("screenTexture", UiRenderTexture->textureHandle());
        compositeShader->SetUniform("screenSize",
            vec2((float)ScreenSize.x, (float)ScreenSize.y));

        // Premultiplied-alpha blend: RGB = src*1 + dst*(1-srcA)
        BgfxStateManager::Reset();
        BgfxStateManager::SetDepthTest(BgfxStateManager::DepthTest::Always);
        BgfxStateManager::SetBlend(BgfxStateManager::Blend::Premultiplied);
        BgfxStateManager::Apply();

        MainRenderer->RenderFullscreenQuad(compositeShader);

    }

    {

        ZoneScopedN("RML UI Pass");

        RmlContext->Render();
    }


    {

		ZoneScopedN("Debug UI");

        if (DebugUiEnabled)
        {
            Level::Current->DevUiUpdate();

            if (GameUpdateSingleThreadPool)
            {
                GameUpdateSingleThreadPool->WaitForFinish();
            }

            bool open = true;

            if (Paused)
            {
                Console::Get().Draw("Console", &open);
                ResourceStatistics::Instance().renderImGui();
            }

            NetworkManager::DrawDebugUi();

            RenderImGui();
        }
    }

    if (LoadingFrames > 0)
    {
        LoadingFrames--;

        LoadingScreenSystem::Draw();
        return;
    }

    {

		ZoneScopedN("Present");

        bgfx::frame();
        ViewIdManager::Reset();
    }
}

void EngineMain::ForceUpdateScreenSize()
{
    int w, h;

    SDL_GetWindowSize(Window, &w, &h);


    ScreenSize.x = w;
    ScreenSize.y = h;

#if __EMSCRIPTEN__

    int width, height;

    EMSCRIPTEN_RESULT result = emscripten_get_canvas_element_size("#canvas", &width, &height);

    if (result == EMSCRIPTEN_RESULT_SUCCESS)
    {
        ScreenSize = ivec2(width, height);
    }
	else
	{
		Logger::Warning("failed to get screen resolution");
	}

#else

#endif // __EMSCRIPTEN__


	if (RmlContext)
		RmlContext->OnResize(ScreenSize.x, ScreenSize.y);
    
}

void EngineMain::FinishRender()
{



}
