#include <Entity.h>

#include <Camera.h>

#include "Player.hpp"

class LevelEndCamera : public Entity
{
public:

	vec3 startPosition = vec3();
	vec3 targetPosition = vec3();

	LevelEndCamera() : Entity()
	{
		auto lookStartEnt = Level::Current->FindEntityWithName("level_end_camera_start");
		auto lookTargetEnt = Level::Current->FindEntityWithName("level_end_camera_target");

		if (lookStartEnt && lookTargetEnt)
		{
			startPosition = lookStartEnt->Position;
			targetPosition = lookTargetEnt->Position;
		}
		else
		{
			startPosition = Camera::position;
			targetPosition = Camera::position + Camera::Forward();
			Logger::Log("LevelEndCamera: Could not find start or target entities, using current camera position and forward direction as fallback.");
		}



		auto player = Level::Current->FindEntityWithName("player");


		Player* playerEntity = Player::Instance;

		if (playerEntity)
		{
			playerEntity->UpdateEnabled = false;
			playerEntity->controller.SetPosition(vec3(0, 100000000, 0));
			playerEntity->controller.SetVelocity(vec3(0, 0, 0));
			playerEntity->Position = vec3(0, 100000000, 0);
			playerEntity->Update();
			playerEntity->AsyncUpdate();
			playerEntity->LateUpdate();



			playerEntity->OnLevelEnd();

		}
	}

	void LateUpdate() override
	{
		Entity::LateUpdate();
		
		Camera::position = startPosition;
		Camera::rotation = MathHelper::FindLookAtRotation(Camera::position, targetPosition);

	}

private:

};

REGISTER_ENTITY(LevelEndCamera, "level_end_camera")