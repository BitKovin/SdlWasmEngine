#include "Crosshair.h"

#include "../../Entities/Player/Player.hpp"

UiCrosshair::UiCrosshair()
{

	float crosshairLineHeight = 3.0f;
	float crosshairLineWidth = 20;

	vec4 crosshairColor = vec4(1.0f, 1.0f, 1.0f, 0.9f);




	dotImage = std::make_shared<UiImage>();
	dotImage->ImagePath = "GameData/textures/ui/crosshair_dot.png";
	dotImage->pivot = vec2(0.5f, 0.5f);
	dotImage->size = vec2(4, 4);
	dotImage->color = crosshairColor;
	AddChild(dotImage);

	leftImage = std::make_shared<UiImage>();
	leftImage->ImagePath = "GameData/textures/ui/crosshair_line.png";
	leftImage->pivot = vec2(1.0f, 0.5f);
	leftImage->position = vec2(-Spread, 0.0f);
	leftImage->size = vec2(crosshairLineWidth, crosshairLineHeight);
	leftImage->color = crosshairColor;
	AddChild(leftImage);

	rightImage = std::make_shared<UiImage>();
	rightImage->ImagePath = "GameData/textures/ui/crosshair_line.png";
	rightImage->pivot = vec2(1.0f, 0.5f);
	rightImage->rotation = 180.0f;
	rightImage->position = vec2(Spread, 0.0f);
	rightImage->size = vec2(crosshairLineWidth, crosshairLineHeight);
	rightImage->color = crosshairColor;
	AddChild(rightImage);

	topImage = std::make_shared<UiImage>();
	topImage->ImagePath = "GameData/textures/ui/crosshair_line.png";
	topImage->pivot = vec2(1.0f, 0.5f);
	topImage->rotation = 90;
	topImage->position = vec2(0.0f, Spread);
	topImage->size = vec2(crosshairLineWidth, crosshairLineHeight);\
	topImage->color = crosshairColor;
	//AddChild(topImage);


	bottomImage = std::make_shared<UiImage>();
	bottomImage->ImagePath = "GameData/textures/ui/crosshair_line.png";
	bottomImage->pivot = vec2(1.0f, 0.5f);
	bottomImage->rotation = -90;
	bottomImage->position = vec2(0.0f, Spread);
	bottomImage->size = vec2(crosshairLineWidth, crosshairLineHeight);
	bottomImage->color = crosshairColor;
	AddChild(bottomImage);


}

UiCrosshair::~UiCrosshair()
{
}

void UiCrosshair::Update()
{


	Player* player = Player::Instance;

	if (player->currentWeapon)
	{

		visible = true;

		Spread = player->currentWeapon->Spread * 20.0f * 75.0f / Camera::FOV + 3;

	}
	else
	{
		visible = false;
	}

	leftImage->position = vec2(-Spread, 0.0f);
	rightImage->position = vec2(Spread, 0.0f);
	topImage->position = vec2(0.0f, Spread);
	bottomImage->position = vec2(0.0f, Spread);

	UiCanvas::Update();
}
