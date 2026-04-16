#pragma once

#include <LoadingScreen/UiLoadingScreenBase.h>
#include <UI/UiImage.hpp>

class UiDefaultLoadingScreen : public UiLoadingScreenBase
{
public:
	UiDefaultLoadingScreen();
	~UiDefaultLoadingScreen();

	std::shared_ptr<UiImage> bgImage = nullptr;
	std::shared_ptr<UiImage> loadingBarImage = nullptr;

	void Update();

private:

};

