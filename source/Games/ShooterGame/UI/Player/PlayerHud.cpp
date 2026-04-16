#include "PlayerHud.hpp"
#include <EngineMain.h>

#include "../../Entities/Player/Player.hpp"

#include "ScoreIndicator.hpp"

#include <UI/UiDropdown.hpp>

PlayerHud::PlayerHud()
{
}

PlayerHud::~PlayerHud()
{
    if (hudCanvas)
        EngineMain::Viewport.RemoveChild(hudCanvas);

    if (ScreenControls)
        EngineMain::Viewport.RemoveChild(ScreenControls);

    hudCanvas.reset();
    ScreenControls.reset();
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

    ammoText = make_shared<UiText>();
    ammoText->position = vec2(-20, -20);
    ammoText->origin = vec2(1, 1);
    ammoText->pivot = vec2(1, 1);
	hudCanvas->AddChild(ammoText);


    hudCanvas->AddChild(text);
    hudCanvas->AddChild(crosshair);



    ScreenControls = make_shared<ScreenMobileControls>();
    EngineMain::Viewport.AddChild(ScreenControls);
    EngineMain::Viewport.AddChild(hudCanvas);

	frameRate = make_shared<UiText>();
    frameRate->origin = vec2(0,0);
	frameRate->position = vec2(10, 10);
	hudCanvas->AddChild(frameRate);

    slots = make_shared<WeaponSlots>();
    slots->player = player;
    slots->origin = vec2(1.0,0.5f);
    slots->pivot = vec2(1.0, 0.5);
    slots->position = vec2(-20,50);
    hudCanvas->AddChild(slots);

    useIndicator = make_shared<UseIndicator>(player);
    useIndicator->origin = vec2(0.5f);

    hudCanvas->AddChild(useIndicator);

    hudCanvas->AddChild(std::make_shared<UiScoreIndicator>());

}

void PlayerHud::Update()
{
    text->text = std::to_string((int)player->Health) + "\n" + to_string(player->stamina);

    ammoText->text = "";
    if (player->currentWeapon)
    {
        if (player->currentWeapon->AmmoType != WeaponAmmoType::None)
        {
            int ammoCount = player->GetAmmo(player->currentWeapon->AmmoType);
            int ammoLimit = player->GetAmmoLimit(player->currentWeapon->AmmoType);
            ammoText->text = to_string(ammoCount) + " / " + to_string(ammoLimit);
		}
    }

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

    UiVerticalBox::Update();

}

void WeaponSlots::Draw()
{

    glm::vec2 pos = position + offset;
    glm::vec2 sz = finalizedSize;

    //UiRenderer::PushMask(pos, sz);

    UiVerticalBox::Draw();//drawing children

    //UiRenderer::PopMask();

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
