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

#include <Settings/VideoSettings.h>

#include <PauseGameManager.hpp>

#include <ItemsDataBase.h>

#include <UI/Pause/UiPauseMenu.h>

#include "Entities/Enemy/DebuffFactory.h"


#include <Analytics/RGameStats/RGameStatsAnalyticsProvider.h>
#include <Analytics/RGameStats/EngineHttpClient.h> 


#include <Network/NetworkManager.h>
#include <Network/ENetTransport.h>
#include <Network/RelayTransport.h>

std::unique_ptr<INetworkTransport> transport;


#ifdef __EMSCRIPTEN__
#include <emscripten.h>

EM_JS(char*, get_input_js, (), {
    let input = window.prompt("Enter room id (or any one char to create server)");
    if (input === null) input = "";

    // Modern way - no need for ALLOC_NORMAL
    let len = lengthBytesUTF8(input) + 1;
    let ptr = _malloc(len);
    stringToUTF8(input, ptr, len);
    return ptr;
    });

#endif

// Then in your function:
std::string getRoomInput() {
    std::string roomId;

#ifdef __EMSCRIPTEN__
    char* jsStr = get_input_js();
    if (jsStr) {
        roomId = jsStr;
        free(jsStr);
    }
#else
    std::cout << "Enter room id (or any one char to create server)\n";
    std::cin >> roomId;
#endif

    return roomId;
}

class GameStart : public Entity
{
public:
	GameStart();
	~GameStart();


    void testHttp();


	void Start()
	{

        auto analyticsProvider = new RGameStatsAnalyticsProvider();
        analyticsProvider->HtmlClient = new EngineHttpClient();
        AnalyticsSystem::Set(analyticsProvider);

		ANALYTICS_SEND_EVENT("reserved_game_started", "");

        LoadConstantAssets();
        LoadingScreenSystem::SetLoadingCanvas(std::make_shared<UiDefaultLoadingScreen>());

        bool isServer = true;
        
        if (false)
        {
#ifdef __EMSCRIPTEN__
            transport = std::make_unique<RelayTransport>("127.0.0.1", /*wsPort=*/5080, 0);
#else
            transport = std::make_unique<RelayTransport>("127.0.0.1", /*tcpPort=*/7777, /*udpPort=*/7778);
#endif

            std::string roomId = getRoomInput();

            if (roomId.size() < 2)
            {
                transport->Host(0, /*maxClients=*/4);
                isServer = true; // Correctly modifies the outer scope variable
            }
            else
            {
                transport->Connect(roomId, 0);
                isServer = false; // Correctly modifies the outer scope variable
            }

          
        }
        else //enet implementation
        {

            transport = make_unique<ENetTransport>();

            auto enetTransport =dynamic_cast<ENetTransport*>(transport.get());

            isServer = !enetTransport->TryConnectOrHost("192.168.0.100", 25666, /*maxClients=*/4);
        }
        

        NetworkManager::BeginLevelLoad(Level::Current);
        NetworkManager::OnLevelLoaded();

        NetworkManager::Init(transport.get(), isServer);

        auto mapArg = EngineMain::MainInstance->Arguments.find("map");

		if (mapArg != EngineMain::MainInstance->Arguments.end() && mapArg->second.size() > 0)
        {

            Logger::Log("opening map from args: " + mapArg->second[0]);

            Level::LoadLevelFromFile(std::string("GameData/maps/") + mapArg->second[0]);
        }
        else
        {
            if(NetworkManager::IsServer())
            Level::LoadLevelFromFile("GameData/maps/lvl1.bsp");
        }
		
        AssetRegistry::GetSkinnedModelFromFile("GameData/models/arms.glb");

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

        if (Input::GetAction("debug_ui_toggle")->Pressed())
        {
            EngineMain::MainInstance->DebugUiEnabled = !EngineMain::MainInstance->DebugUiEnabled;
        }

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
            UiManager::UiScale = 1.3f;
        }

        if (Input::MouseDelta != vec2())
        {
            UiManager::UiScale = 1;
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

	bool wasPaused = false;

    std::shared_ptr<UiPauseMenu> pauseMenu;

    void UpdatePaused()
    {

        Input::LockCursor = PauseGameManager::GetGamePaused() == false;

		EngineMain::MainInstance->Paused = PauseGameManager::isGameAnyPaused();

        if (PauseGameManager::GetGamePaused())
        {
            if (wasPaused == false)
            {

                pauseMenu = std::make_shared<UiPauseMenu>();

                EngineMain::MainInstance->Viewport.AddChild(pauseMenu);

				//RmlUiContext::Main->PushModal(pauseMenuDoc);
            }
        }
        else
        {
            if (pauseMenu != nullptr)
            {

                pauseMenu->RemoveFromParent();
                pauseMenu = nullptr;

				//RmlUiContext::Main->RemoveFromModalFromStack(pauseMenuDoc);
            }
        }

		wasPaused = PauseGameManager::GetGamePaused();

	}

    void LoadConstantAssets()
    {
		AssetRegistry::LoadingConstantAssets = true;

		PreloadEntityType("info_player_start");
        PreloadEntityType("player");
        PreloadEntityType("npc_zombie");
        PreloadEntityType("npc_human_axe");
        PreloadEntityType("npc_human_gun");
        PreloadEntityType("bullet");

        auto entityTypes = LevelObjectFactory::instance().GetRegistry();

        for (auto& type : entityTypes)
        {
            auto& className = type.first;

			if (StringHelper::StartsWith(className, "weapon_"))
            {
                PreloadEntityType(className);
            }

        }

		AssetRegistry::LoadingConstantAssets = false;
    }

private:

    static inline bool startedGame = false;

};

#include "Entities/Npc/NpcBase.h"

GameStart::GameStart()
{

	Localisation::SetLanguage("ua");

	PauseGameManager::SetGamePaused(false);
	PauseGameManager::SetGamePausedGameplay(false);

    NpcBase::globalPhraceDelay = Delay();

    LoadConstantAssets();

	UpdateWhenPaused = true;

    Spawn("npcSimulationManager");

	auto debuffs = DebuffFactory::Instance().GetRegisteredDebuffNames();

    for (auto& name : debuffs)
    {
		auto debuff = DebuffFactory::Instance().CreateDebuff(name);

		AssetRegistry::GetTextureFromFile(debuff->iconPath);

    }

    if (startedGame) return;

    UiRenderer::AddFallbackFont("GameData/fonts/NotoSans.ttf");
    UiRenderer::AddFallbackFont("GameData/fonts/NotoSerifJP.ttf");
    UiRenderer::AddFallbackFont("GameData/fonts/NotoSansSC.ttf");

    GameSettings::Instance().LoadFromFile();
    GameSettings::Instance().ApplyAll();

    startedGame = true;

    Input::AddAction("forward")->AddKeyboardKey(SDL_KeyCode::SDLK_w);
    Input::AddAction("backward")->AddKeyboardKey(SDL_KeyCode::SDLK_s);
    Input::AddAction("left")->AddKeyboardKey(SDL_KeyCode::SDLK_a);
    Input::AddAction("right")->AddKeyboardKey(SDL_KeyCode::SDLK_d);

    Input::AddAction("crouch")->AddKeyboardKey(SDL_KeyCode::SDLK_c);

    Input::AddAction("jump")->AddKeyboardKey(SDL_KeyCode::SDLK_SPACE)->AddButton(GamepadButton::A);

    Input::AddAction("dash")->AddKeyboardKey(SDL_KeyCode::SDLK_LSHIFT)->AddButton(GamepadButton::LeftShoulder);

    InputActionRegistry::Register("forward", "Move Forward", "Movement");
	InputActionRegistry::Register("backward", "Move Backward", "Movement");
	InputActionRegistry::Register("left", "Move Left", "Movement");
	InputActionRegistry::Register("right", "Move Right", "Movement");
	InputActionRegistry::Register("crouch", "Crouch", "Movement");
	InputActionRegistry::Register("jump", "Jump", "Movement");
	InputActionRegistry::Register("dash", "Dash", "Movement");

    Input::AddAction("attack")->AddButton(GamepadButton::RightTrigger)->AddMouseButton(0);
    Input::AddAction("attack2")->AddButton(GamepadButton::LeftTrigger)->AddMouseButton(1);

    Input::AddAction("attack3")->AddButton(GamepadButton::Y)->AddKeyboardKey(SDL_KeyCode::SDLK_r);

	InputActionRegistry::Register("attack", "Attack", "Combat");
	InputActionRegistry::Register("attack2", "Offhand Attack", "Combat");
    InputActionRegistry::Register("attack3", "Secondary Offhand Attack", "Combat");

    Input::AddAction("qSave")->AddKeyboardKey(SDL_KeyCode::SDLK_F6);
    Input::AddAction("qLoad")->AddKeyboardKey(SDL_KeyCode::SDLK_F7);

    Input::AddAction("interact")->AddKeyboardKey(SDL_KeyCode::SDLK_f);

	InputActionRegistry::Register("interact", "Interact", "General");
	InputActionRegistry::Register("qSave", "Quick Save", "General");
	InputActionRegistry::Register("qLoad", "Quick Load", "General");


    Input::AddAction("cameraView")->AddKeyboardKey(SDL_KeyCode::SDLK_v);



#if __EMSCRIPTEN__

    Input::AddAction("pause")->AddKeyboardKey(SDL_KeyCode::SDLK_BACKQUOTE)->AddButton(GamepadButton::Back);

#else
    Input::AddAction("pause")->AddKeyboardKey(SDL_KeyCode::SDLK_ESCAPE)->AddButton(GamepadButton::Back)->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_AC_BACK));
#endif

    Input::AddAction("slot1")->AddKeyboardKey(SDL_KeyCode::SDLK_1);
    Input::AddAction("slot2")->AddKeyboardKey(SDL_KeyCode::SDLK_2);
    Input::AddAction("slot3")->AddKeyboardKey(SDL_KeyCode::SDLK_3);
    Input::AddAction("slot4")->AddKeyboardKey(SDL_KeyCode::SDLK_4);
    Input::AddAction("slot5")->AddKeyboardKey(SDL_KeyCode::SDLK_5);
    Input::AddAction("slot6")->AddKeyboardKey(SDL_KeyCode::SDLK_6);
    Input::AddAction("lastSlot")->AddKeyboardKey(SDL_KeyCode::SDLK_q);

	InputActionRegistry::Register("slot1", "Select Slot 1", "Inventory");
	InputActionRegistry::Register("slot2", "Select Slot 2", "Inventory");
	InputActionRegistry::Register("slot3", "Select Slot 3", "Inventory");
	InputActionRegistry::Register("slot4", "Select Slot 4", "Inventory");
	InputActionRegistry::Register("slot5", "Select Slot 5", "Inventory");
	InputActionRegistry::Register("slot6", "Select Slot 6", "Inventory");
	InputActionRegistry::Register("lastSlot", "Select Last Slot", "Inventory");

    Input::AddAction("inventory")->AddKeyboardKey(SDL_KeyCode::SDLK_TAB)->AddButton(GamepadButton::DPadUp);

    Input::AddAction("slotTest")->AddKeyboardKey(SDL_KeyCode::SDLK_t);

    Input::AddAction("reload_shaders")->AddKeyboardKey(SDL_KeyCode::SDLK_F3);

    Input::AddAction("block")->AddKeyboardKey(SDL_KeyCode::SDLK_LCTRL);

    Input::AddAction("slotMelee");// ->AddKeyboardKey(SDL_KeyCode::SDLK_f);


    Input::AddAction("debug_ui_toggle")->AddKeyboardKey(SDL_KeyCode::SDLK_F2);

	Input::AddAction("ui_confirm")->AddKeyboardKey(SDL_KeyCode::SDLK_RETURN)->AddButton(GamepadButton::A);
    Input::AddAction("ui_cancel")->AddKeyboardKey(SDL_KeyCode::SDLK_ESCAPE)->AddButton(GamepadButton::B)->AddKeyboardKey(SDL_GetScancodeFromKey(SDLK_AC_BACK));
    Input::AddAction("ui_down")->AddKeyboardKey(SDL_KeyCode::SDLK_DOWN)->AddButton(GamepadButton::DPadDown);
	Input::AddAction("ui_up")->AddKeyboardKey(SDL_KeyCode::SDLK_UP)->AddButton(GamepadButton::DPadUp);
    Input::AddAction("ui_left")->AddKeyboardKey(SDL_KeyCode::SDLK_LEFT)->AddButton(GamepadButton::DPadLeft);
	Input::AddAction("ui_right")->AddKeyboardKey(SDL_KeyCode::SDLK_RIGHT)->AddButton(GamepadButton::DPadRight);




    //Input::AddAction("dbg_simulate")->AddKeyboardKey(SDL_KeyCode::SDLK_j);

    ItemsDataBase::LoadItemsDataBase();

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