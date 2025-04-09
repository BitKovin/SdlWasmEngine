#include "../Entity.hpp"

#include "../Input.h"

#include "../MathHelper.hpp"

#include "../Camera.h"

#include "../Physics.h"

#include "../DebugDraw.hpp"

#include <algorithm>   // for std::clamp
#include <cmath>       // for std::max

#include "../Navigation/Navigation.hpp"

#include "../SkeletalMesh.hpp"

class Player : public Entity
{

private:

    float maxSpeed = 7;
    float maxSpeedAir = 2;
    float acceleration = 90;
    float airAcceleration = 30;

    vec3 velocity = vec3(0);

    glm::vec3 Friction(glm::vec3 vel, float factor = 60.0f) {
        vel = MathHelper::XZ(vel);
        float length = glm::length(vel);

        // Avoid division by zero: if length is positive, normalize; otherwise return zero vector.
        glm::vec3 direction = (length > 0.0f) ? glm::normalize(vel) : glm::vec3(0.0f);

        length -= factor * Time::DeltaTimeF;
        length = std::max(0.0f, length);

        return direction * length;
    }

    glm::vec3 UpdateGroundVelocity(glm::vec3 withDir, glm::vec3 vel) {
        vel = MathHelper::XZ(vel);
        vel = Friction(vel);

        // Project current velocity onto the direction
        float currentSpeed = glm::dot(vel, withDir);

        // Clamp the additional speed so that it does not exceed what can be accelerated in the frame.
        float addSpeed = glm::clamp(maxSpeed - currentSpeed, 0.0f, acceleration * Time::DeltaTimeF);

        if (false) {
            if (currentSpeed + addSpeed > maxSpeed)
                addSpeed = maxSpeed - currentSpeed;
        }

        return vel + addSpeed * withDir;
    }

    glm::vec3 UpdateAirVelocity(glm::vec3 wishdir, glm::vec3 vel) {
        vel = MathHelper::XZ(vel);

        float currentSpeed = glm::dot(vel, wishdir);
        float wishspeed = maxSpeedAir;
        float addSpeed = wishspeed - currentSpeed;

        if (addSpeed <= 0.0f) {
            return vel;
        }

        float accelspeed = airAcceleration * Time::DeltaTimeF * wishspeed;

        if (accelspeed > addSpeed) {
            accelspeed = addSpeed;
        }

        return vel + accelspeed * wishdir;
    }

    void Jump()
    {
        LeadBody->SetLinearVelocity(JPH::Vec3(velocity.x, 9.5, velocity.z));
    }

    bool CheckGroundAt(vec3 location)
    {
        auto result = Physics::SphereTrace(location, location - vec3(0, 0.95, 0), 1, BodyType::GroupCollisionTest, {LeadBody});

        return result.hasHit;

    }

    vec3 testStart;

    SkeletalMesh viewmodel;

public:
	Player(){}
	~Player(){}

	float Speed = 5;

	void Start()
	{
		LeadBody = Physics::CreateCharacterBody(this, Position, 0.5, 1.8, 70);
        Physics::SetGravityFactor(LeadBody, 3);

        viewmodel.LoadFromFile("GameData/testViewmodel.glb");
        viewmodel.ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");

        viewmodel.PlayAnimation("idle");

        viewmodel.IsViewmodel = true;

        Drawables.push_back(&viewmodel);

	}

    bool OnGround = false;

    dtObstacleRef playerObstacle = 0;

	void Update()
	{

        

        //NavigationSystem::RemoveObstacle(playerObstacle);
        //playerObstacle = NavigationSystem::CreateObstacleBox(Position + vec3(2,0,0) - vec3(0.4, 1, 0.2), Position + vec3(2, 0, 0) + vec3(0.4, 1, 0.2));

        OnGround = CheckGroundAt(Position);

		Camera::rotation.y += Input::MouseDelta.x;
		Camera::rotation.x -= Input::MouseDelta.y;

		vec2 input = Input::GetLeftStickPosition();

		if (Input::GetAction("forward")->Holding())
			input += vec2(0,1);

		if (Input::GetAction("backward")->Holding())
			input += vec2(0, -1);

		if (Input::GetAction("left")->Holding())
			input += vec2(-1, 0);

		if (Input::GetAction("right")->Holding())
			input += vec2(1, 0);

		vec3 right = MathHelper::GetRightVector(Camera::rotation);

		vec3 forward = MathHelper::GetForwardVector(vec3(0,Camera::rotation.y,0));


		if (length(input) > 1)
			input = normalize(input);

		vec3 movement = input.x * right + input.y * forward;

		Physics::Activate(LeadBody);

        velocity = FromPhysics(LeadBody->GetLinearVelocity());

        if (OnGround)
        {
            velocity = UpdateGroundVelocity(movement, velocity);
        }
        else
        {
            velocity = UpdateAirVelocity(movement, velocity);
        }
        

        velocity.y = LeadBody->GetLinearVelocity().GetY();

		LeadBody->SetLinearVelocity(ToPhysics(velocity));

        if (Input::GetAction("test")->Pressed())
        {
            testStart = Position;
        }

        if(OnGround)
        if (Input::GetAction("jump")->Pressed())
        {
            Jump();
            //NavigationSystem::CreateObstacleBox(Position - vec3(1.1, 3, 1.1), Position + vec3(1.1, 3, 1.1));

            DebugDraw::Line(Position, testStart);

            auto path = NavigationSystem::FindSimplePath(Position, testStart);

            path.insert(path.begin(), Position);

            DebugDraw::Path(path, 5);

        }

		Camera::position = Position + vec3(0,0.7,0);

        viewmodel.Update();
        viewmodel.Position = Camera::position;
        viewmodel.Rotation = Camera::rotation;

	}

};