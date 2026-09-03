#include <Entity.h>

#include <EngineMain.h>
#include <Input.h>

class GameStart : public Entity
{
public:
	GameStart();
	~GameStart();

	void Start()
	{

		Input::AddAction("forward")->AddKeyboardKey(SDL_KeyCode::SDLK_w);
		Input::AddAction("backward")->AddKeyboardKey(SDL_KeyCode::SDLK_s);
		Input::AddAction("left")->AddKeyboardKey(SDL_KeyCode::SDLK_a);
		Input::AddAction("right")->AddKeyboardKey(SDL_KeyCode::SDLK_d);

		Input::AddAction("rmb")->AddMouseButton(1);

		Level::LoadLevelFromFile("GameData/maps/blank.bsp");



		EngineMain::MainInstance->DebugUiEnabled = true;

		//EngineMain::MainInstance->asyncGameUpdate = false;

	}

private:

};

GameStart::GameStart()
{

	Spawn("freecamera");
	Spawn("skeletal_editor");

}

GameStart::~GameStart()
{
}

REGISTER_ENTITY(GameStart,"gamestart")