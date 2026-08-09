#include "PlayerHud.hpp"
#include <EngineMain.h>
#include <cstdio>

#include "../../Entities/Player/Player.hpp"

#include "ScoreIndicator.hpp"

#include <UI/UiDropdown.hpp>

namespace
{
    // "3" / "3.0"-style formatting for the stamina readout.
    std::string FormatOneDecimal(float v)
    {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f", v);
        return std::string(buf);
    }
}

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

    // ── Retro text status bar: STAMINA / HEALTH / AMMO ─────────────────────
    // Each column is a fixed size so the three stay evenly spaced no matter
    // how many digits a value has, and the whole row is pivot-centered so it
    // stays centered on any aspect ratio (canvas is 1080 tall, variable
    // width). See UI/HudStatusBar.hpp and UI/HudStatElement.hpp.
    statusBar = make_shared<HudStatusBar>();
    hudCanvas->AddChild(statusBar);

    const vec2 statColumnSize = vec2(170.f, 100.f);
    staminaStat = make_shared<HudStatElement>("${PLAYER_HUD_STAMINA}", statColumnSize, vec4(0.32f, 0.32f, 0.44f, 1.f));
    statusBar->AddChild(staminaStat);

    healthStat = make_shared<HudStatElement>("${PLAYER_HUD_HEALTH}", statColumnSize, vec4(0.45f, 0.21f, 0.21f, 1.f));
    statusBar->AddChild(healthStat);

    ammoStat = make_shared<HudStatElement>("${PLAYER_HUD_AMMO}", statColumnSize, vec4(0.64f, 0.60f, 0.48f, 1.f));
    statusBar->AddChild(ammoStat);

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

    //hudCanvas->AddChild(std::make_shared<UiScoreIndicator>());

}

void PlayerHud::Update()
{
    staminaStat->SetValue(FormatOneDecimal(player->stamina));
	staminaStat->ProgressBar->Progress = player->stamina / 3.0f;
    healthStat->SetValue(std::to_string((int)player->Health));
	healthStat->ProgressBar->Progress = player->Health / player->MaxHealth;

    ammoStat->SetValue("");
    if (player->currentWeapon)
    {
        if (player->currentWeapon->GetAmmoType() != WeaponAmmoType::None)
        {
            int ammoCount = player->GetAmmo(player->currentWeapon->GetAmmoType());
            int ammoLimit = player->GetAmmoLimit(player->currentWeapon->GetAmmoType());
            ammoStat->SetValue(to_string(ammoCount));// +" / " + to_string(ammoLimit));
			ammoStat->ProgressBar->Progress = ammoLimit > 0 ? (float)ammoCount / (float)ammoLimit : 0.0f;
		}
    }

    crosshair->visible = !useIndicator->visible;

	//frameRate->text = "FPS: " + to_string((int)(1.0f / Time::DeltaTimeF));

}

void WeaponSlots::Update()
{
    // 1. Gather the correct slot data FIRST based on the active mode
    std::vector<WeaponSlotData> targetSlots = player->weaponSlots;

    if (player->GetWeaponSystemMode() == WeaponSystemMode::Inventory)
    {
        targetSlots.clear();
        int index = 0;

        for (auto item : player->GetInventory())
        {
            if (item.mainWeaponData.className != "")
            {
                item.mainWeaponData.slot = index;
                targetSlots.push_back(item.mainWeaponData);
            }
            else if (item.offhandWeaponData.className != "")
            {
                item.offhandWeaponData.slot = index;
                targetSlots.push_back(item.offhandWeaponData);
            }
            index++;
        }
    }

    // 2. Check if the active equipment changed based on mode
    bool equipmentChanged = false;
    if (player->GetWeaponSystemMode() == WeaponSystemMode::Inventory)
    {
        equipmentChanged = (oldMainUUID != player->currentMainWeaponUUID ||
            oldOffhandUUID != player->currentOffhandWeaponUUID);
    }
    else
    {
        equipmentChanged = (oldSlot != player->currentSlot);
    }

    // 3. Early exit if neither the items nor the equipped weapons changed
    // We check targetSlots instead of player->weaponSlots!
    if (oldSlots == targetSlots && !equipmentChanged)
    {
        UiVerticalBox::Update();
        return;
    }

    // --- Rebuild UI ---
    children.clear();

    for (WeaponSlotData& data : targetSlots)
    {
        if (data.className == "") continue;

        auto img = make_shared<UiButton>();
        img->size = vec2(120, 120);

        bool equipped = false;
        if (player->GetWeaponSystemMode() == WeaponSystemMode::Inventory)
        {
            equipped = (data.inventoryUUID == player->currentMainWeaponUUID ||
                data.inventoryUUID == player->currentOffhandWeaponUUID);
        }
        else
        {
            equipped = (data.slot == player->currentSlot);
        }

        if (equipped)
        {
            img->color = vec4(1, 0.5, 0.5, 1);
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
        text->origin = vec2(0, 1);
        text->pivot = vec2(0, 1);
        text->text = to_string(data.slot + 1);
        text->fontSize = 50;
        text->position = vec2(5, -5);

        img->AddChild(text);
        AddChild(img);
    }

    // 4. Update all cached states to match current frame
    oldSlots = targetSlots;
    oldSlot = player->currentSlot;
    oldMainUUID = player->currentMainWeaponUUID;
    oldOffhandUUID = player->currentOffhandWeaponUUID;

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
