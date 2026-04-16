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

	if (Input::IsTouchEventHolding(TrackingTouch) == false)
	{
		TrackingTouch = 0;
	}

	if (TrackingTouch == 0)
	{

		for (auto& touch : TouchEvents)
		{

			if (touch.released && FastTapAction != "" && tapWindowDelay.Wait())
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
					}

				}

				touchStart = touch.position;

				TrackingTouch = touch.id;

				tapWindowDelay.AddDelay(0.2);

			}

		}

		

	}



	//vec2 center = position + offset + MoveAreaSize / 2.0f;

	vec2 relativeTouchPosition = Input::GetTouchEventPosition(TrackingTouch) - touchStart;

	if (TrackingTouch == 0)
	{
		relativeTouchPosition = vec2(0);

		color = vec4(0);

	}
	else
	{
		color = vec4(1);
	}

	InputPosition = relativeTouchPosition / MoveAreaSize * 2.0f * vec2(1,-1);

	if (length(InputPosition) > 1)
	{
		InputPosition = normalize(InputPosition);
	}

	bgImage->position = touchStart - (position + offset);
	stickImage->position = touchStart + InputPosition * vec2(1, -1) / 2.0f * MoveAreaSize - (position + offset);


}
