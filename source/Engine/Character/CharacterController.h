#pragma once
#include "../Physics.h"
#include "../Entity.h"
#include <vector>
#include <algorithm>

enum class CharacterControllerMovementQuality
{

	Player = 0,// Huge amount of shape casts, but very smooth movement. Use for player characters.
	NpcGeneric = 1, // One shape cast, few line casts when on slope.
	NpcLowQuality = 2 // One line cast, even less line casts, line casts only on steep slope.

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
	Body* sensorBody = nullptr;

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

	// Registry of all live CharacterControllers. Populated in Init(), removed in destructor.
	static std::vector<CharacterController*> s_allControllers;

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

	// BodyIDs of character bodies we have an active AddIgnorePair with.
	// This controller is always the "top" character in the pair.
	std::vector<BodyID> activeIgnorePairs;

	bool CanStandUp();

	void UpdateGroundCheck(bool& hitsGround, float& calculatedCharacterHeight, bool& canStand, vec3& walkableNormal, vec3& notWalkableNormal);

	bool CheckGroundAt(vec3 location, float radius, float& height, bool& canStand, vec3& normal, const Body** hitBody);

	// Detects characters we are standing on, manages AddIgnorePair/RemoveIgnorePair,
	// and applies horizontal separation impulses to both bodies.
	void UpdateCharacterStacking(float deltaTime);

};
