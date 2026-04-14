#pragma once
#include "../Physics.h"
#include "../Entity.h"

enum class CharacterControllerMovementQuality
{

	Player = 0,
	NpcGeneric = 1

};

class CharacterController
{
public:
	CharacterController();
	~CharacterController();

	void Init(Entity* owner, vec3 position, float radius = 0.5, float height = 1.8, float mass = 30);

	void Destroy();

	void Update(float deltaTime);

	vec3 GetPosition();
	vec3 GetSmoothPosition();
	void SetPosition(vec3 position);
	void SetSmoothPosition(vec3 position);

	void UpdateSmoothPosition(float deltaTime);

	vec3 GetVelocity();
	void SetVelocity(vec3 vel);

	Body* body = nullptr;

	const Body* standingOnBody = nullptr;

	float gravity = 25;

	float stepHeight = 0.4;

	float height = 1.8;
	float radius = 0.4;

	vec3 currentGroundNormal = vec3(0.0f,1.0f,0.0f);

	float groundMaxAngle = 40;

	float stepSmoothingSpeed = 15.0f;

	bool onGround = false;

	float cameraHeightStanding = 1.6f;
	float cameraHeightCrouching = 0.8f;
	float crouchHeight = 0.80f;

	CharacterControllerMovementQuality movementQuality = CharacterControllerMovementQuality::Player;

	float GetCameraHeight();

	void Crouch();
	void UnCrouch();

	static float GroundAngleRad(const glm::vec3& normal);

	/// Same, but returns degrees.
	static float GroundAngleDeg(const glm::vec3& normal);

	float heightSmoothOffset = 0;

	void SetCollisionMask(BodyType mask)
	{
		Physics::SetCollisionMask(body, mask);
	}

	void SetBodyType(BodyType type)
	{
		Physics::SetBodyType(body, type);
	}

	bool isCrouched = false;

private:

	Entity* owner = nullptr;
	float standingHeight = 1.8f;

	float currentCameraHeight = 0.0f;
	float targetCameraHeight = 0.0f;

	const Body* lastStandingOnBody = nullptr;
	glm::vec3 baseLocalAttachPoint = glm::vec3(0.0f);
	glm::vec3 prevAttachWorldPos = glm::vec3(0.0f);
	glm::quat prevBaseRotation = glm::quat(1, 0, 0, 0);
	glm::vec3 prevBasePosition = glm::vec3(0.0f);
	glm::vec3 lastPlatformVelocity = glm::vec3(0.0f);
	bool wasOnPlatform = false;

	bool CanStandUp();

	void UpdateGroundCheck(bool& hitsGround, float& calculatedCharacterHeight, bool& canStand, vec3& notWalkableNormal);

	bool CheckGroundAt(vec3 location, float radius, float& height, bool& canStand, vec3& normal, const Body** hitBody);

	//vec3 velocity = vec3(0);

};