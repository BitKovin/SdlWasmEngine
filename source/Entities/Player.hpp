#include "../Entity.hpp"

#include "../Input.h"

#include "../MathHelper.hpp"

#include "../Camera.h"

class Player : public Entity
{
public:
	Player(){}
	~Player(){}

	float Speed = 5;

	void Update()
	{
		Camera::rotation.y += Input::MouseDelta.x;
		Camera::rotation.x -= Input::MouseDelta.y;

		vec2 input = vec2();

		if (Input::GetAction("forward")->Holding())
			input += vec2(0,1);

		if (Input::GetAction("backward")->Holding())
			input += vec2(0, -1);

		if (Input::GetAction("left")->Holding())
			input += vec2(-1, 0);

		if (Input::GetAction("right")->Holding())
			input += vec2(1, 0);

		vec3 right = MathHelper::GetRightVector(Camera::rotation);

		vec3 forward = MathHelper::GetForwardVector(Camera::rotation);


		if (length(input) > 1)
			input = normalize(input);

		vec3 movement = input.x * right + input.y * forward;

		Position += movement * Speed * Time::DeltaTimeF;



		Camera::position = Position;

	}

private:

};