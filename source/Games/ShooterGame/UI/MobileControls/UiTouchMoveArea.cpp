#include "UiTouchMoveArea.h"

UiTouchMoveArea::UiTouchMoveArea()
{
	color = vec4(0);

	HitCheck = true;
}

UiTouchMoveArea::~UiTouchMoveArea()
{
}

vec2 UiTouchMoveArea::GetTouchMovement()
{
	return Input::GetTouchEventDelta(TrackingTouch);
}

void UiTouchMoveArea::Update()
{
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
				TrackingTouch = touch.id;
				touchStartPosition = touch.position;
				isTracking = true;
				isDragging = false;

				break;
			}
		}
	}
	else
	{
		if (isDragging == false && length(Input::GetTouchEventPosition(TrackingTouch) - touchStartPosition) > TapMaxMovement)
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

		// Fallback: The touch ended without us seeing a matching "released" event this frame
		// (e.g. interrupted by the OS). Drop it silently rather than firing a tap.
		// We only hit this if isTracking is STILL true (meaning no explicit release was found above).
		if (isTracking && Input::IsTouchEventHolding(TrackingTouch) == false)
		{
			isTracking = false;
			isDragging = false;
			TrackingTouch = 0;
		}
	}
}
