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

        Input::AddAction("rmb")->RMB = true;
        Input::AddAction("growth")->AddKeyboardKey(SDL_KeyCode::SDLK_LALT);
        Input::AddAction("submit")->AddKeyboardKey(SDL_KeyCode::SDLK_RETURN);

        //Level::OpenLevel("GameData/maps/blank.bsp");

        Spawn("freecamera");
        Spawn("editorCore");
        //Spawn("skeletal_editor");

        EngineMain::MainInstance->DebugUiEnabled = true;

        //EngineMain::MainInstance->asyncGameUpdate = false;

    }

private:

};

GameStart::GameStart()
{


}

GameStart::~GameStart()
{
}

REGISTER_ENTITY(GameStart,"gamestart")