#include <Entity.h>

#include <World/WorldOrientationManager.h>

class TestGravityController : public Entity
{
public:
	
	void Update() override
	{

		Entity::Update();

		WorldOrientationManager::SetUpVector(MathHelper::GetUpVector(Rotation));
		WorldOrientationManager::UpdateWorld();

		float t = Time::GameTime;

		float pitch =
			glm::radians(3.0f) * sin(t * 0.25f)
			+ glm::radians(1.2f) * sin(t * 0.9f)
			+ glm::radians(0.5f) * sin(t * 2.3f);

		float roll =
			glm::radians(6.0f) * sin(t * 0.45f)
			+ glm::radians(2.0f) * sin(t * 1.4f)
			+ glm::radians(0.7f) * sin(t * 2.8f);

		glm::quat rot =
			glm::angleAxis(roll, glm::vec3(0, 0, 1)) *
			glm::angleAxis(pitch, glm::vec3(1, 0, 0));

		Rotation = MathHelper::ToYawPitchRoll(rot);

		//Rotation = vec3(0,0,30);

	}

private:

};

REGISTER_ENTITY(TestGravityController, "testGravityController");