#pragma once

#include <UI/UiViewport.hpp>
#include <UI/UiCanvas.hpp>
#include <UI/UiText.hpp>
#include <UI/UiHorizontalBox.hpp>
#include <UI/UiButton.hpp>
#include <UI/UiImage.hpp>
#include "Crosshair.h"
#include <UI/UiVideo.hpp>
#include <UI/UiVerticalBox.hpp>
#include <UI/UiProgressBar.hpp>
#include <Entity.h>
#include <Entities/Player/Weapons/WeaponBase.h>

#include "ScreenMobileControls.h"

class Player;

class WeaponSlots : public UiVerticalBox
{
public:
	
	int oldSlot = -1;
	std::vector<WeaponSlotData> oldSlots;

	Player* player = nullptr;

	void Update();

	void Draw() override;

private:

};

class UseIndicator : public UiCanvas
{
public:
	UseIndicator(Player* player);

	void Update() override;

private:

	std::shared_ptr<UiImage> useIcon;
	std::shared_ptr<UiProgressBar> progressBar;
	std::shared_ptr<UiText> text;

	Player* playerRef = nullptr;

};

class StaminaBar : public UiImage
{

public:

	StaminaBar();

	std::shared_ptr<UiProgressBar> staminaFill;
	std::shared_ptr<UiImage> shadowImage;

	float stamina = 1.0f;

	void Update() override;

};

class PlayerHud
{
public:
	PlayerHud();
	~PlayerHud();

	void Init(Player* player);

	void Update();

	void SetVisible(bool visible)
	{
		hudCanvas->visible = visible;
		ScreenControls->visible = visible;
	}

	std::shared_ptr<ScreenMobileControls> ScreenControls;

private:

	Player* player = nullptr;


	std::shared_ptr<UiCanvas> hudCanvas;

	std::shared_ptr<WeaponSlots> slots;

	std::shared_ptr<UiButton> img;
	std::shared_ptr<UiText> text;
	std::shared_ptr<UiText> ammoText;
	std::shared_ptr<UiCrosshair> crosshair;

	std::shared_ptr<UiText> frameRate;

	std::shared_ptr<UseIndicator> useIndicator;


	std::shared_ptr<UiImage> healthBar;
	std::shared_ptr<UiImage> armorBar;
	std::shared_ptr<UiImage> playerStatusContainer;

	std::shared_ptr<StaminaBar> staminaBar1;
	std::shared_ptr<StaminaBar> staminaBar2;
	std::shared_ptr<StaminaBar> staminaBar3;

};