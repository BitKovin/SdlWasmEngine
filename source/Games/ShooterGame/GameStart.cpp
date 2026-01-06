#include <Entity.h>
#include <Input.h>

#include <BSP/Quake3Bsp.h>
#include <UI/UiManager.h>
#include "Entities/TestBsp.h"
#include <LoadingScreen/LoadingScreenSystem.h>
#include "UI/LoadingScreen/UiDefaultLoadingScreen.h"
#include <UI/RmlUi/RmlUiContext.h>
#include <UI/RmlUi/RmlUiEvents.h>
#include <EngineMain.h>

#include "Entities/Enemy/DebuffFactory.h"

class GameStart : public Entity
{
public:
	GameStart();
	~GameStart();

	Rml::ElementDocument* pauseMenuDoc = nullptr;
    Rml::ElementDocument* settingsMenuDoc = nullptr;


    void testHttp();

	void Start()
	{

        LoadingScreenSystem::SetLoadingCanvas(std::make_shared<UiDefaultLoadingScreen>());

        auto mapArg = EngineMain::MainInstance->Arguments.find("map");

		if (mapArg != EngineMain::MainInstance->Arguments.end() && mapArg->second.size() > 0)
        {

            Logger::Log("opening map from args: " + mapArg->second[0]);

            Level::LoadLevelFromFile(std::string("GameData/maps/") + mapArg->second[0]);
        }
        else
        {
            Level::LoadLevelFromFile("GameData/maps/test.bsp");
        }
		

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

        //Level::Current->AddEntity(new TestBsp());

	}

    void Finalize()
    {
        if (Input::GetAction("reload_shaders")->Pressed())
        {
            AssetRegistry::ReloadShaders();

			EngineMain::MainInstance->RmlContext->ReloadAllSttyles();

        }
    }

    void Update()
    {


        UpdatePaused();

        if (Input::IsScrenTouched)
        {
            UiManager::UiScale = 2;
        }

        if (Input::MouseDelta != vec2())
        {
            UiManager::UiScale = 1;
        }

        if (Input::GetAction("debug_ui_toggle")->Pressed())
        {
            EngineMain::MainInstance->DebugUiEnabled = !EngineMain::MainInstance->DebugUiEnabled;
        }

    }

    void UpdateDebugUI()
    {

        ImGui::Begin("game debug");

        if (ImGui::Button("test http request"))
        {
            testHttp();
        }

        ImGui::End();

    }

    void UpdatePaused()
    {
        if (EngineMain::MainInstance->Paused)
        {
            if (!pauseMenuDoc->IsVisible())
            {
				RmlUiContext::Main->PushModal(pauseMenuDoc);
            }
        }
        else
        {
            if (pauseMenuDoc != nullptr && pauseMenuDoc->IsVisible())
            {
				RmlUiContext::Main->RemoveFromModalFromStack(pauseMenuDoc);
            }
        }
	}

    void ConstructPauseMenu()
    {
        
		pauseMenuDoc = RmlUiContext::Main->LoadDocument("GameData/ui/pause.rml");

        RmlUiEvents::onClick(pauseMenuDoc, "backBtn", []()
            {
				EngineMain::MainInstance->Paused = false;
            });
        RmlUiEvents::onClick(pauseMenuDoc, "optionsBtn", [&]()
            {
				RmlUiContext::Main->PushModal(settingsMenuDoc);
            });


		settingsMenuDoc = RmlUiContext::Main->LoadDocument("GameData/ui/settings.rml");
        RmlUiEvents::onClick(settingsMenuDoc, "backBtn", []()
            {
                RmlUiContext::Main->PopModal();
            });

	}

    void ConstructMenus()
    {
        ConstructPauseMenu();
	}

private:

    static inline bool startedGame = false;

};

#include "Entities/Npc/NpcBase.h"

GameStart::GameStart()
{

    NpcBase::globalPhraceDelay = Delay();


	UpdateWhenPaused = true;
    ConstructMenus();

    Spawn("npcSimulationManager");

	auto debuffs = DebuffFactory::Instance().GetRegisteredDebuffNames();

    for (auto& name : debuffs)
    {
		auto debuff = DebuffFactory::Instance().CreateDebuff(name);

		AssetRegistry::GetTextureFromFile(debuff->iconPath);

    }

    if (startedGame) return;

    startedGame = true;

    Input::AddAction("forward")->AddKeyboardKey(SDL_KeyCode::SDLK_w);
    Input::AddAction("backward")->AddKeyboardKey(SDL_KeyCode::SDLK_s);
    Input::AddAction("left")->AddKeyboardKey(SDL_KeyCode::SDLK_a);
    Input::AddAction("right")->AddKeyboardKey(SDL_KeyCode::SDLK_d);

    Input::AddAction("jump")->AddKeyboardKey(SDL_KeyCode::SDLK_SPACE)->AddButton(GamepadButton::A);

    Input::AddAction("attack")->AddButton(GamepadButton::RightTrigger)->LMB = true;
    Input::AddAction("attack2")->AddButton(GamepadButton::LeftTrigger)->RMB = true;

    Input::AddAction("qSave")->AddKeyboardKey(SDL_KeyCode::SDLK_F6);
    Input::AddAction("qLoad")->AddKeyboardKey(SDL_KeyCode::SDLK_F7);

    Input::AddAction("cameraView")->AddKeyboardKey(SDL_KeyCode::SDLK_v);

    Input::AddAction("dash")->AddKeyboardKey(SDL_KeyCode::SDLK_LSHIFT)->AddButton(GamepadButton::LeftShoulder);

#if __EMSCRIPTEN__

    Input::AddAction("pause")->AddKeyboardKey(SDL_KeyCode::SDLK_BACKQUOTE)->AddButton(GamepadButton::Back);

#else
    Input::AddAction("pause")->AddKeyboardKey(SDL_KeyCode::SDLK_ESCAPE)->AddButton(GamepadButton::Back);
#endif

    Input::AddAction("slot1")->AddKeyboardKey(SDL_KeyCode::SDLK_1);
    Input::AddAction("slot2")->AddKeyboardKey(SDL_KeyCode::SDLK_2);
    Input::AddAction("slot3")->AddKeyboardKey(SDL_KeyCode::SDLK_3);
    Input::AddAction("slot4")->AddKeyboardKey(SDL_KeyCode::SDLK_4);
    Input::AddAction("slot5")->AddKeyboardKey(SDL_KeyCode::SDLK_5);
    Input::AddAction("lastSlot")->AddKeyboardKey(SDL_KeyCode::SDLK_q);

    Input::AddAction("slotTest")->AddKeyboardKey(SDL_KeyCode::SDLK_t);

    Input::AddAction("reload_shaders")->AddKeyboardKey(SDL_KeyCode::SDLK_F3);

    Input::AddAction("slotMelee")->AddKeyboardKey(SDL_KeyCode::SDLK_f);

    Input::AddAction("debug_ui_toggle")->AddKeyboardKey(SDL_KeyCode::SDLK_UP);

	Input::AddAction("ui_confirm")->AddKeyboardKey(SDL_KeyCode::SDLK_RETURN)->AddButton(GamepadButton::A);
    Input::AddAction("ui_cancel")->AddKeyboardKey(SDL_KeyCode::SDLK_ESCAPE)->AddButton(GamepadButton::B);
    Input::AddAction("ui_down")->AddKeyboardKey(SDL_KeyCode::SDLK_DOWN)->AddButton(GamepadButton::DPadDown);
	Input::AddAction("ui_up")->AddKeyboardKey(SDL_KeyCode::SDLK_UP)->AddButton(GamepadButton::DPadUp);
    Input::AddAction("ui_left")->AddKeyboardKey(SDL_KeyCode::SDLK_LEFT)->AddButton(GamepadButton::DPadLeft);
	Input::AddAction("ui_right")->AddKeyboardKey(SDL_KeyCode::SDLK_RIGHT)->AddButton(GamepadButton::DPadRight);

    Input::AddAction("dbg_simulate")->AddKeyboardKey(SDL_KeyCode::SDLK_j);

}

GameStart::~GameStart()
{
}


#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <http/http_client.h>
void GameStart::testHttp()
{

    http_client::Request req;
    req.method = http_client::Method::GET;
    req.url = "http://httpbin.org/image/png";
    req.headers["Accept"] = "application/json";

    http_client::RequestId id = start_request(req);

    std::cout << "Request started. Polling until response...\n";

    // Simulate frame loop: poll until done
    while (!http_client::is_done(id)) {
        std::cout << ".";
        std::cout.flush();
#ifdef __EMSCRIPTEN__
        // In browser we can't sleep; just break after some spins
        //emscripten_sleep(100);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
    }
    std::cout << "\n";

    http_client::Response resp;
    if (http_client::get_response(id, resp)) {
        if (resp.status == http_client::Status::Success) 
        {
            std::cout << "HTTP " << resp.status_code << "\n";
            std::cout << "Body:\n" << resp.body << "\n"; // print first 300 chars

            std::ofstream out("image.png", std::ios::binary);
            out.write(resp.body.data(), resp.body.size());
            out.close();


        }
        else {
            std::cout << "Request failed: " << resp.error << "\n";
        }
    }
    else {
        std::cout << "No response available.\n";
    }

    // Second test: POST with JSON body
    http_client::Request postReq;
    postReq.method = http_client::Method::POST;
    postReq.url = "http://httpbin.org/post";
    postReq.headers["Content-Type"] = "application/json";
    postReq.body = R"({"hello":"world"})";

    http_client::RequestId postId = start_request(postReq);
    std::cout << "\nPOST started...\n";

    while (!http_client::is_done(postId)) {
#ifdef __EMSCRIPTEN__
        // In browser we can't sleep; just break after some spins
        //emscripten_sleep(100);
#else
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
    }

    http_client::Response postResp;
    if (get_response(postId, postResp)) {
        if (postResp.status == http_client::Status::Success) {
            std::cout << "POST HTTP " << postResp.status_code << "\n";
            std::cout << "POST Body:\n" << postResp.body << "\n";
        }
        else {
            std::cout << "POST failed: " << postResp.error << "\n";
        }
    }

}

REGISTER_ENTITY(GameStart,"gamestart")