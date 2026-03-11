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

#include <Profiling/ResourceStatistics.hpp>

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
        printf("failed to get screen resolution\n");
    }



#else

    int w, h;

    SDL_GetWindowSize(Window, &w, &h);


    ScreenSize.x = w;
    ScreenSize.y = h;

#endif // __EMSCRIPTEN__

	if (ScreenSize != oldScreenSize)
    {
        if(RmlContext)
		    RmlContext->OnResize(ScreenSize.x, ScreenSize.y);
    }

}

void EngineMain::ToggleFullscreen()
{

    auto context = SDL_GL_GetCurrentContext();

    Uint32 FullscreenFlag = SDL_WINDOW_FULLSCREEN_DESKTOP;
    bool IsFullscreen = SDL_GetWindowFlags(Window) & FullscreenFlag;



    // Toggle fullscreen
    SDL_SetWindowFullscreen(Window, IsFullscreen ? 0 : FullscreenFlag);

    // Rebind the context to the window
    SDL_GL_MakeCurrent(Window, context);

    // Ensure double-buffer swap is clean
    SDL_GL_SwapWindow(Window);

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

    Input::AddAction("click")->LMB = true;


}

void EngineMain::Init(std::vector<std::string> args)
{

#ifdef DISTRIBUTION
    DebugUiEnabled = false;
#endif // DISTRIBUTION


    Arguments = ParseCommands(args);


    LevelObjectFactory::instance().registerDefaults();
    BehaviorTree::RegisterTypes();

    UpdateScreenSize();

    printf("init\n");

    MainThreadPool = new ThreadPool();

    MainThreadPool->Start(ThreadPool::GetNumThreadsForThreadPool());

    SoundManager::Initialize();

    Time::Init();

    ConsoleDefaultCommands::RegisterAll();

    printf("initPhys\n");
    Physics::Init();
    printf("initPhys\n");

    FileSystemEngine::Init();

    MainRenderer = new Renderer();

    UiRenderer::Init();

	RmlContext = new RmlUiContext(Window, 800, 600, true);
	RmlContext->Initialize();
	RmlUiContext::Main = RmlContext;

    ParticleEmitter::InitBilboardVaoIfNeeded();

    InitInputs();

    Level::Current = new Level();

    ImStartFrame();
	RenderImGui();

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
    

    if (frame == 5) //some platforms require it
    {
        ForceUpdateScreenSize(); //hack to fix some rendering issues that happen on some platforms
        initGame();
    }


    if (DebugUiEnabled)
    {
        ImStartFrame();
    }
    

    bool loadedlevel = Level::LoadPendingLevel();


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

    Viewport.Update();
    Viewport.UpdateChildrenOffsetRecursive();
    Viewport.FinalizeChildren();

    Input::UpdateMouse();

    
    if (asyncGameUpdate) 
    {
        gameUpdateFuture = std::async(std::launch::async, &EngineMain::GameUpdate, this);
    }
    else 
    {

        // Run GameUpdate on the main thread.
        GameUpdate();
    }

    Render();

    if (asyncGameUpdate == false)
    {
        FinishFrame();
    }

    FinishRender();

    if (asyncGameUpdate)
    {
        if (gameUpdateFuture.valid()) {
            // If it's not done yet, wait (or you could choose to skip/warn).
            if (gameUpdateFuture.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
                gameUpdateFuture.wait();
            }
        }

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

    NavigationSystem::Update();
	if (Paused == false)
		Physics::Simulate();
    Physics::UpdateDebugDraw();

    Level::Current->UpdatePhysics();

    AiPerceptionSystem::Update();

    Level::Current->Update(Paused);

    Level::Current->AsyncUpdate(Paused);

    Level::Current->LateUpdate(Paused);

    if(SimulatingGameTicks == false)
        SoundManager::Update();

}

void EngineMain::Render()
{
    const uint16_t UI_VIEW = 0;
    const uint16_t WORLD_VIEW = 1;
    const uint16_t COMPOSITE_VIEW = 2;

    ivec2 uiResolution = ivec2(
        UiManager::GetScaledUiHeight() * Camera::AspectRatio,
        UiManager::GetScaledUiHeight()
    );

    /* ============================================================
       CREATE / RESIZE UI RENDER TARGET
       ============================================================ */

    if (UiRenderTexture == nullptr ||
        UiRenderTexture->width() != uiResolution.x ||
        UiRenderTexture->height() != uiResolution.y)
    {
        delete UiRenderTexture;

        UiRenderTexture = new RenderTexture(
            uiResolution.x,
            uiResolution.y,
            TextureFormat::RGBA8
        );

        UiRenderTexture->SetName("UiRenderTexture");
    }

    /* ============================================================
       UI PASS → render to transparent RT
       ============================================================ */

    bgfx::setViewFrameBuffer(UI_VIEW, UiRenderTexture->frameBufferHandle());

    bgfx::setViewRect(
        UI_VIEW,
        0,
        0,
        (uint16_t)uiResolution.x,
        (uint16_t)uiResolution.y
    );

    bgfx::setViewClear(
        UI_VIEW,
        BGFX_CLEAR_COLOR,
        0x00000000, // transparent
        1.0f,
        0
    );

    // UI renderer should submit draw calls with:
    // BGFX_STATE_BLEND_ONE | BGFX_STATE_BLEND_INV_SRC_ALPHA

    Viewport.Draw();
    UiRenderer::EndFrame();

    /* ============================================================
       WORLD PASS
       ============================================================ */

    bgfx::setViewFrameBuffer(WORLD_VIEW, BGFX_INVALID_HANDLE);

    bgfx::setViewRect(
        WORLD_VIEW,
        0,
        0,
        (uint16_t)ScreenSize.x,
        (uint16_t)ScreenSize.y
    );

    bgfx::setViewClear(
        WORLD_VIEW,
        BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH,
        0x000000ff,
        1.0f,
        0
    );

    MainRenderer->RenderLevel(Level::Current);

    /* ============================================================
       COMPOSITE UI OVER SCENE
       ============================================================ */

    bgfx::setViewRect(
        COMPOSITE_VIEW,
        0,
        0,
        (uint16_t)ScreenSize.x,
        (uint16_t)ScreenSize.y
    );

    auto fullscreenShader = ShaderManager::GetShaderProgram(
        "fullscreen_vertex",
        "fxaa_simple"
    );

    fullscreenShader->SetTexture("screenTexture", UiRenderTexture->textureHandle());

    fullscreenShader->SetUniform(
        "screenSize",
        vec2((float)ScreenSize.x, (float)ScreenSize.y)
    );

    uint64_t state =
        BGFX_STATE_WRITE_RGB |
        BGFX_STATE_WRITE_A |
        BGFX_STATE_BLEND_ONE |
        BGFX_STATE_BLEND_INV_SRC_ALPHA;

    bgfx::setState(state);

    MainRenderer->RenderFullscreenQuad(fullscreenShader);


    RmlContext->Render();

    if (DebugUiEnabled)
    {
        Level::Current->DevUiUpdate();

        bool open = true;

        if (Paused)
        {
            Console::Get().Draw("Console", &open);
            ResourceStatistics::Instance().renderImGui();
        }


        RenderImGui();
    }


    if (LoadingFrames > 0)
    {
        LoadingFrames--;

        LoadingScreenSystem::Draw();
        return;
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
        printf("failed to get screen resolution\n");
    }

#else

#endif // __EMSCRIPTEN__


	if (RmlContext)
		RmlContext->OnResize(ScreenSize.x, ScreenSize.y);
    
}

void EngineMain::FinishRender()
{

	bgfx::frame();

}
