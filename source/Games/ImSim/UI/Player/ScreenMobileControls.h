#include "../MobileControls/UiJoystick.h"
#include "../MobileControls/UiTouchMoveArea.h"
#include <UI/UiCanvas.hpp>
#include <EngineMain.h>

class ScreenMobileControls : public UiCanvas
{
public:
	
	shared_ptr<UiTouchMoveArea> TouchArea;
	shared_ptr<UiJoystick> Joystick;

	ScreenMobileControls()
	{
		TouchArea = make_shared<UiTouchMoveArea>();

		TouchArea->origin = vec2(1, 0.5);
		TouchArea->pivot = vec2(1, 0.5);
		TouchArea->DoubleTapAction = "attack";

		AddChild(TouchArea);

		Joystick = make_shared<UiJoystick>();
		Joystick->origin = vec2(0, 1);
		Joystick->pivot = vec2(0, 1);
		Joystick->size = vec2(200,200);
		Joystick->position = vec2(0,0);
		Joystick->FastTapAction = "jump";
		AddChild(Joystick);

	}

	void Update()
	{

		UiElement::Update();

		TouchArea->size = EngineMain::MainInstance->Viewport.GetSize() * vec2(0.5, 1);
		Joystick->size = EngineMain::MainInstance->Viewport.GetSize() * vec2(0.4, 0.5);
	}

private:

};
