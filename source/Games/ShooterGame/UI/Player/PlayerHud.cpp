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

    useIndicator = make_shared<UseIndicator>(player);
    useIndicator->origin = vec2(0.5f);

    hudCanvas->AddChild(useIndicator);

}

void PlayerHud::Update()
{
    text->text = std::to_string((int)player->Health) + "\n" + to_string(player->stamina);

    crosshair->visible = !useIndicator->visible;

	//frameRate->text = "FPS: " + to_string((int)(1.0f / Time::DeltaTimeF));

}

void WeaponSlots::Update()
{

    if (oldSlot == player->currentSlot && oldSlots == player->weaponSlots)
    {
        //UiHorizontalBox::Update();
        //return;
    }

    children.clear();

	std::vector<WeaponSlotData> slots = player->weaponSlots;

	int currentSlot = player->currentSlot;

	if (player->GetWeaponSystemMode() == WeaponSystemMode::Inventory)
    {

		//currentSlot = player->currentInventoryIndex;

        slots.clear();

		int index = 0;

        for (auto item : player->GetInventory())
        {
            if (item.mainWeaponData.className != "")
            {
                item.mainWeaponData.slot = index;
                slots.push_back(item.mainWeaponData);
            }
            else if (item.offhandWeaponData.className != "")
            {
				item.offhandWeaponData.slot = index;
                slots.push_back(item.offhandWeaponData);
            }

            index++;
        }
    }

    for (WeaponSlotData& data : slots)
    {
        if (data.className == "") continue;

        auto img = make_shared<UiButton>();
        img->size = vec2(120,120);

		bool equipped = (data.slot == currentSlot);

        if (player->GetWeaponSystemMode() == WeaponSystemMode::Inventory)
        {

            equipped = data.inventoryUUID == player->currentMainWeaponUUID || data.inventoryUUID == player->currentOffhandWeaponUUID;
            
        }

        if (equipped)
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

UseIndicator::UseIndicator(Player* player)
{

    ScaleToParent = false;

    useIcon = make_shared<UiImage>();
    useIcon->origin = vec2(0.0f);
    useIcon->pivot = vec2(0.5f);
    useIcon->size = vec2(40);

    progressBar = make_shared<UiProgressBar>();
    progressBar->position = vec2(60, 90);
    progressBar->size = vec2(200,15);
    progressBar->color = vec4(0.5f,0.2f,0.5f,1);

    text = make_shared<UiText>();
    text->text = "Press F to use\nHold F to use alt";
    text->fontSize = 36;

    text->position = vec2(15,15);

    AddChild(useIcon);
    AddChild(progressBar);
    AddChild(text);

    playerRef = player;



}

void UseIndicator::Update()
{

    UiCanvas::Update();

    visible = playerRef->currentInteractionObject != nullptr;

    progressBar->Progress = playerRef->interactionProgress;

}
