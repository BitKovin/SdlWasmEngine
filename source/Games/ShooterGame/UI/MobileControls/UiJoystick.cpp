#include "UiJoystick.h"

UiJoystick::UiJoystick()
{
	HitCheck = true;

	bgImage = make_shared<UiImage>();
	bgImage->ImagePath = "GameData/textures/ui/circle.png";
	bgImage->origin = vec2(0);
	bgImage->pivot = vec2(0.5);
	bgImage->color = vec4(1,0.7,0.7,0.3);
	stickImage = make_shared<UiImage>();
	stickImage->pivot = vec2(0.5);
	stickImage->origin = vec2(0);
	stickImage->ImagePath = "GameData/textures/ui/circle.png";
	stickImage->color = vec4(1, 0.7, 0.7, 0.3);

	AddChild(bgImage);
	AddChild(stickImage);

}

UiJoystick::~UiJoystick()
{
}

vec2 UiJoystick::GetTouchMovement()
{
	return Input::GetTouchEventDelta(TrackingTouch);
}

void UiJoystick::Update()
{
	bgImage->size = MoveAreaSize;
	stickImage->size = MoveAreaSize / 2.0f;

	UiElement::Update();

	// A tap that already happened but is still waiting to see whether a second tap
	// follows in time to upgrade it to a double tap. Checked every frame, independent
	// of whatever touch is currently being tracked below.
	if (hasPendingTap && tapWindowDelay.Wait() == false)
	{
		hasPendingTap = false;

		if (FastTapAction != "")
		{
			auto action = Input::GetAction(FastTapAction);

			if (action)
			{
				action->SimulatePressed();
			}
		}
	}

	if (!isTracking)
	{
		for (auto& touch : TouchEvents)
		{
			if (touch.pressed && touch.id > 1)
			{
				touchStart = touch.position;
				TrackingTouch = touch.id;
				isTracking = true;
				isDragging = false;

				break;
			}
		}
	}
	else
	{
		if (isDragging == false && length(Input::GetTouchEventPosition(TrackingTouch) - touchStart) > TapMaxMovement)
		{
			isDragging = true;
		}

		for (auto& touch : TouchEvents)
		{
			if (touch.released && touch.id == TrackingTouch)
			{
				if (isDragging == false)
				{
					if (hasPendingTap && tapWindowDelay.Wait())
					{
						// A qualifying tap arrived within the window of the previous one - that's a double tap.
						hasPendingTap = false;

						if (DoubleTapAction != "")
						{
							auto action = Input::GetAction(DoubleTapAction);

							if (action)
							{
								action->SimulatePressed();
							}
						}
					}
					else
					{
						// Could still turn into the first half of a double tap - hold off firing
						// the single tap until the window passes without a follow-up tap.
						hasPendingTap = true;
						tapWindowDelay.AddDelay(DoubleTapWindow);
					}
				}

				isTracking = false;
				isDragging = false;
				TrackingTouch = 0;

				break;
			}
		}

		// Fallback for OS interruptions without explicit releases.
		if (isTracking && Input::IsTouchEventHolding(TrackingTouch) == false)
		{
			isTracking = false;
			isDragging = false;
			TrackingTouch = 0;
		}
	}

	// While within the tap threshold (or not tracking at all) the stick stays centered -
	// this doubles as the joystick's dead zone.
	vec2 relativeTouchPosition = vec2(0);

	if (isTracking && isDragging)
	{
		relativeTouchPosition = Input::GetTouchEventPosition(TrackingTouch) - touchStart;
	}

	color = isTracking ? vec4(1) : vec4(0);

	InputPosition = relativeTouchPosition / MoveAreaSize * 2.0f * vec2(1, -1);

	if (length(InputPosition) > 1)
	{
		InputPosition = normalize(InputPosition);
	}

	bgImage->position = touchStart - (position + offset);
	stickImage->position = touchStart + InputPosition * vec2(1, -1) / 2.0f * MoveAreaSize - (position + offset);
}