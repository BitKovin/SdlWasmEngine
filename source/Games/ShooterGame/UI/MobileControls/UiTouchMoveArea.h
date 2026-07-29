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

	// True while we're currently following a touch (tap candidate or drag, either way).
	bool IsTracking() const { return isTracking; }

	// True once the currently tracked touch has moved past TapMaxMovement (i.e. it's a drag, not a tap).
	bool IsDragging() const { return isDragging; }

	// Id of the touch being followed. Only meaningful while IsTracking() is true - check that,
	// not a comparison against this value, to know whether a touch is currently tracked.
	int TrackingTouch = 0;

	string DoubleTapAction = "";
	string FastTapAction = ""; 

	// How far (in local units) a touch may drift and still count as a tap rather than a drag.
	float TapMaxMovement = 24.0f;

	// Max time between one tap's release and the next tap's release for the pair to count as a double tap.
	float DoubleTapWindow = 0.3f;

private:

	Delay tapWindowDelay;

	bool isTracking = false;
	bool isDragging = false;
	bool hasPendingTap = false;

	vec2 touchStartPosition = vec2(0);

};
