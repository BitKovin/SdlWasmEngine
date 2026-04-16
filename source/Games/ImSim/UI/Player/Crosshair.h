#pragma once

#include <UI/UiImage.hpp>
#include <UI/UiCanvas.hpp>

class UiCrosshair : public UiCanvas
{
public:
	UiCrosshair();
	~UiCrosshair();

	float Spread = 5.0f; //in pixels

	std::shared_ptr<UiImage> dotImage;
	std::shared_ptr<UiImage> leftImage;
	std::shared_ptr<UiImage> rightImage;
	std::shared_ptr<UiImage> topImage;
	std::shared_ptr<UiImage> bottomImage;

	void Update() override;

private:

};

