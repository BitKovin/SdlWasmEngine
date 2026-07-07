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

	playerStatusContainer = make_shared<UiImage>();
	hudCanvas->AddChild(playerStatusContainer);

	playerStatusContainer->ImagePath = "GameData/textures/ui/hud/status.png";
	playerStatusContainer->size = vec2(392, 317);
	playerStatusContainer->origin = vec2(0.0f, 1.0f);
	playerStatusContainer->pivot = vec2(0.0f, 1.0f);

	playerStatusContainer->color = vec4(vec3(.85f),0.9f);

	healthBar = make_shared<UiImage>();
	playerStatusContainer->AddChild(healthBar);
	healthBar->ImagePath = "GameData/textures/ui/hud/progressbar.png";
	healthBar->size = vec2(302, 37);
	healthBar->position = vec2(28, 160);
	healthBar->color = vec4(1, 0.2, 0.2, 1);


    armorBar = make_shared<UiImage>();
    playerStatusContainer->AddChild(armorBar);
    armorBar->ImagePath = "GameData/textures/ui/hud/progressbar.png";
    armorBar->size = vec2(302, 37);
    armorBar->position = vec2(28, 92);
    armorBar->color = vec4(1.1, 0.7, 0.0, 1);

	staminaBar1 = make_shared<StaminaBar>();
	playerStatusContainer->AddChild(staminaBar1);
	staminaBar1->origin = vec2(0, 1);
	staminaBar1->pivot = vec2(0, 1);
	staminaBar1->position = vec2(40, -15);

    staminaBar2 = make_shared<StaminaBar>();
    playerStatusContainer->AddChild(staminaBar2);
    staminaBar2->origin = vec2(0, 1);
    staminaBar2->pivot = vec2(0, 1);
    staminaBar2->position = vec2(40 + 95, -15);

    staminaBar3 = make_shared<StaminaBar>();
    playerStatusContainer->AddChild(staminaBar3);
    staminaBar3->origin = vec2(0, 1);
    staminaBar3->pivot = vec2(0, 1);
    staminaBar3->position = vec2(40 + 95 * 2, -15);

    //hudCanvas->AddChild(std::make_shared<UiScoreIndicator>());

}

void PlayerHud::Update()
{
    text->text = "";// std::to_string((int)player->Health) + "\n" + to_string(player->stamina);

    ammoText->text = "";
    if (player->currentWeapon)
    {
        if (player->currentWeapon->GetAmmoType() != WeaponAmmoType::None)
        {
            int ammoCount = player->GetAmmo(player->currentWeapon->GetAmmoType());
            int ammoLimit = player->GetAmmoLimit(player->currentWeapon->GetAmmoType());
            ammoText->text = to_string(ammoCount) + " / " + to_string(ammoLimit);
		}
    }

    crosshair->visible = !useIndicator->visible;

    healthBar->size = vec2(glm::max(0.0f, glm::min(1.0f, player->Health / player->MaxHealth)) * 302, 37);

	float stamina = player->stamina;

	float stamina1 = glm::clamp(stamina, 0.0f, 1.0f);
	float stamina2 = glm::clamp(stamina - 1.0f, 0.0f, 1.0f);
	float stamina3 = glm::clamp(stamina - 2.0f, 0.0f, 1.0f);

	staminaBar1->stamina = stamina1;
	staminaBar2->stamina = stamina2;
	staminaBar3->stamina = stamina3;


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

StaminaBar::StaminaBar()
{

	ImagePath = "GameData/textures/ui/hud/stamina_bg.png";
    size = vec2(81, 77) * 1.05f;

	staminaFill = make_shared<UiProgressBar>();
	AddChild(staminaFill);
	staminaFill->BackgroundImage = "GameData/textures/ui/hud/stamina_fill.png";
    staminaFill->BackgroundColor = vec4(0);
	staminaFill->ProgressImage = "GameData/textures/ui/hud/stamina_fill.png";
	staminaFill->size = vec2(58, 55) * 1.05f;
    staminaFill->rotation = -90;
    staminaFill->position = vec2(-1,0);

	staminaFill->color = vec4(vec3(0.8f), 1);

    staminaFill->pivot = vec2(0.5f);
	staminaFill->origin = vec2(0.5f);

	shadowImage = make_shared<UiImage>();
    AddChild(shadowImage);
    shadowImage->ImagePath = ImagePath;
	shadowImage->size = size * 1.05f;
	shadowImage->color = vec4(vec3(0), 0.4f);
	shadowImage->position = vec2(1, 1);

    stamina = 0.5f;

}

void StaminaBar::Update()
{

    staminaFill->Progress = stamina;

    UiImage::Update();
}
