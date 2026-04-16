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


	if (Input::IsTouchEventHolding(TrackingTouch) == false)
	{
		TrackingTouch = 0;
	}

	if (TrackingTouch == 0)
	{

		holdingDoubleTap = false;

		for (auto& touch : TouchEvents)
		{

			if (touch.released && touch.id == TrackingTouch && FastTapAction != "" && tapWindowDelay.Wait())
			{
				auto action = Input::GetAction(FastTapAction);

				if (action)
				{
					action->SimulatePressed();
				}
			}

			if (touch.pressed && touch.id > 1)
			{

				if (tapWindowDelay.Wait() && DoubleTapAction != "")
				{
					auto action = Input::GetAction(DoubleTapAction);

					if (action)
					{
						action->SimulatePressed();
						holdingDoubleTap = true;
					}

				}

				TrackingTouch = touch.id;

				tapWindowDelay.AddDelay(0.2);

			}

		}
	}
	else
	{
		if (holdingDoubleTap)
		{
			auto action = Input::GetAction(DoubleTapAction);

			if (action)
			{
				action->SimulateHolding();
			}
		}
	}

}
