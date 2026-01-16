#include "PlayerHud.hpp"
#include <EngineMain.h>

#include "../../Entities/Player/Player.hpp"

PlayerHud::PlayerHud()
{
}

PlayerHud::~PlayerHud()
{
    EngineMain::Viewport.RemoveChild(hudCanvas);
}

void PlayerHud::Init(Player* playerRef)
{


	player = playerRef;

    hudCanvas = make_shared<UiCanvas>();
    crosshair = make_shared<UiCrosshair>();

    crosshair->origin = vec2(0.5f, 0.5f);
	hudCanvas->AddChild(crosshair);

    text = make_shared<UiText>();

    text->position = vec2(20, -20);

    text->origin = vec2(0, 1);
    text->pivot = vec2(0, 1);

    text->text = std::to_string((int)player->Health);


    hudCanvas->AddChild(text);
    //hudCanvas->AddChild(crosshair);



    ScreenControls = make_shared<ScreenMobileControls>();
    EngineMain::Viewport.AddChild(ScreenControls);
    EngineMain::Viewport.AddChild(hudCanvas);

	frameRate = make_shared<UiText>();
    frameRate->origin = vec2(0,0);
	frameRate->position = vec2(10, 10);
	hudCanvas->AddChild(frameRate);

    slots = make_shared<WeaponSlots>();
    slots->player = player;
    slots->origin = vec2(0.5,0);
    slots->pivot = vec2(0.5, 0);
    slots->position = vec2(0,20);
    hudCanvas->AddChild(slots);

}

void PlayerHud::Update()
{
    text->text = std::to_string((int)player->Health);
	//frameRate->text = "FPS: " + to_string((int)(1.0f / Time::DeltaTimeF));

}

void WeaponSlots::Update()
{

    if (oldSlot == player->currentSlot && oldSlots == player->weaponSlots)
    {
        UiHorizontalBox::Update();
        return;
    }

    children.clear();

    for (WeaponSlotData data : player->weaponSlots)
    {
        if (data.className == "") continue;

        auto img = make_shared<UiButton>();
        img->size = vec2(120,120);

        if (data.slot == player->currentSlot)
        {
            img->color = vec4(1,0.5,0.5,1);
        }
        else
        {
            img->color = vec4(1, 0.8, 0.8, 0.8);
        }

        img->OnlyTouch = true;
        img->OnlyNotPaused = true;

        img->onClick = [this, data]() {
            player->SwitchToSlot(data.slot);
            };


        auto text = make_shared<UiText>();
        text->origin = vec2(0,1);
        text->pivot = vec2(0, 1);
        text->text = to_string(data.slot + 1);
        text->fontSize = 50;
        text->position = vec2(5,-5);

        img->AddChild(text);

        AddChild(img);

    }

    oldSlot = player->currentSlot;
    oldSlots = player->weaponSlots;

    UiHorizontalBox::Update();

}
