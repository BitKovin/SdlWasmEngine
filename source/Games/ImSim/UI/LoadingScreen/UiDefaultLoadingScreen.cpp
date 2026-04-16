#include "UiDefaultLoadingScreen.h"

UiDefaultLoadingScreen::UiDefaultLoadingScreen()
{

	bgImage = make_shared<UiImage>();
	bgImage->ImagePath = "GameData/cat.png";

	loadingBarImage = make_shared<UiImage>();
	loadingBarImage->ImagePath = "GameData/cat.png";
	loadingBarImage->color = vec4(1,0,0,1);
	loadingBarImage->pivot = vec2(0, 1);
	loadingBarImage->origin = vec2(0, 1);
	loadingBarImage->position = vec2(30,-50);
	loadingBarImage->size = vec2(100,10);

	AddChild(bgImage);
	AddChild(loadingBarImage);

}

UiDefaultLoadingScreen::~UiDefaultLoadingScreen()
{
}

void UiDefaultLoadingScreen::Update()
{

	bgImage->size = GetSize();
	loadingBarImage->size.x = (bgImage->size.x - 60) * LoadingProgress;

	UiLoadingScreenBase::Update();

}
