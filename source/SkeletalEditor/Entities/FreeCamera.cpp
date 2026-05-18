#include <Entity.h>
#include <Camera.h>
#include <Input.h>
#include <imgui.h>
#include <imgui/imgui_internal.h>

class FreeCamera : public Entity
{
public:
	FreeCamera();
	~FreeCamera();

	void Update()
	{

		Input::LockCursor = Input::GetAction("rmb")->Holding();

		if (Input::GetAction("rmb")->Holding() == false) return;

		if(ImGui::GetCurrentContext()->CurrentWindow)
			ImGui::SetKeyboardFocusHere(-1);

		vec2 input = Input::GetLeftStickPosition();

		if (Input::GetAction("forward")->Holding())
			input += vec2(0, 1);

		if (Input::GetAction("backward")->Holding())
			input += vec2(0, -1);

		if (Input::GetAction("left")->Holding())
			input += vec2(-1, 0);

		if (Input::GetAction("right")->Holding())
			input += vec2(1, 0);

		Camera::rotation.y += Input::MouseDelta.x;
		Camera::rotation.x -= Input::MouseDelta.y;

		vec3 right = MathHelper::GetRightVector(Camera::rotation);

		vec3 forward = Camera::Forward();


		vec3 movement = input.x * right + input.y * forward;

		Camera::position += movement * Time::DeltaTimeF * 3.0f;
	}

private:

};

REGISTER_ENTITY(FreeCamera, "freecamera")

FreeCamera::FreeCamera()
{
}

FreeCamera::~FreeCamera()
{
}