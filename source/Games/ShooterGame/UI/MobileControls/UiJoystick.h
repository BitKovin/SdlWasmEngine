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

	// True while we're currently following a touch (tap candidate or drag, either way).
	bool IsTracking() const { return isTracking; }

	// True once the currently tracked touch has moved past TapMaxMovement (i.e. it's a drag, not a tap).
	bool IsDragging() const { return isDragging; }

	// Id of the touch being followed. Only meaningful while IsTracking() is true - check that,
	// not a comparison against this value, to know whether a touch is currently tracked.
	int TrackingTouch = 0;

	vec2 InputPosition = vec2(0);

	string DoubleTapAction = ""; 
	string FastTapAction = "";

	vec2 MoveAreaSize = vec2(200);

	// How far (in local units) a touch may drift and still count as a tap rather than a drag.
	// Also acts as the joystick's dead zone.
	float TapMaxMovement = 15.0f;

	// Max time between one tap's release and the next tap's release for the pair to count as a double tap.
	float DoubleTapWindow = 0.3f;

private:

	shared_ptr<UiImage> bgImage;
	shared_ptr<UiImage> stickImage;

	Delay tapWindowDelay;

	bool isTracking = false;
	bool isDragging = false;
	bool hasPendingTap = false;

	vec2 touchStart = vec2();

};
