#pragma once

#include <UI/UiImage.hpp>
#include <Delay.hpp>

class UiJoystick : public UiElement
{
public:
	UiJoystick();
	~UiJoystick();

	vec2 GetTouchMovement();

	void Update();

	int TrackingTouch = 0;

	vec2 InputPosition = vec2(0);

	string DoubleTapAction = "";
	string FastTapAction = "";

	vec2 MoveAreaSize = vec2(200);

private:

	shared_ptr<UiImage> bgImage;
	shared_ptr<UiImage> stickImage;

	Delay tapWindowDelay;

	vec2 touchStart = vec2();

};