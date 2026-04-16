#pragma once
#include <UI/UiImage.hpp>
#include <glm.h>
#include <Delay.hpp>

class UiTouchMoveArea : public UiImage
{
public:
	UiTouchMoveArea();
	~UiTouchMoveArea();

	vec2 GetTouchMovement();

	void Update();

	int TrackingTouch = 0;

	string DoubleTapAction = "";
	string FastTapAction = "";

private:

	Delay tapWindowDelay;

	bool holdingDoubleTap = false;

};

