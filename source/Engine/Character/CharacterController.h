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
	virtual ~CharacterController();

	virtual void Init(Entity* owner, vec3 position, float radius = 0.5, float height = 1.8, float mass = 30);

	void Destroy();

	virtual void Update(float deltaTime);

	vec3 GetPosition();
	vec3 GetSmoothPosition();
	void SetPosition(vec3 position);
	void SetSmoothPosition(vec3 position);

	vec3 GetSmoothOffset();

	void UpdateSmoothPosition(float deltaTime);

	virtual vec3 GetVelocity();
	virtual void SetVelocity(vec3 vel);

	Body* body = nullptr;
	Body* sensorBody = nullptr;

	const Body* standingOnBody = nullptr;
	Entity* standingOnEntity = nullptr;

	float gravity = 25;

	float stepHeight = 0.4;

	bool allowSleep = false;

	float height = 1.8;
	float radius = 0.4;

	vec3 currentGroundNormal = vec3(0.0f, 1.0f, 0.0f);

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

	// ── Ladder mode ───────────────────────────────────────────────────────────
	// When true, CharacterController::Update() skips manual gravity integration
	// so the ladder code has sole control over vertical velocity.
	// Call SetLadderMode(true/false) — it also rebuilds the physics body with
	// stepHeight = 0 so the capsule bottom sits flush with the player's feet
	// and can't catch on platform corners during descent.
	bool suppressGravity = false;
	bool isOnLadder = false;
	void SetLadderMode(bool enabled);

	// Registry of all live CharacterControllers. Populated in Init(), removed in destructor.
	static std::vector<CharacterController*> s_allControllers;

protected:

	// Exposed to subclasses (e.g. QuakeCharacterController) so alternate
	// movement implementations can reuse the existing ground-sampling,
	// character-stacking, and camera-smoothing logic instead of
	// reimplementing it. Platform-attach state below is protected (not
	// private) for the same reason: it's tightly coupled to the base
	// class's own velocity-integration order, so a subclass with a
	// different integration order (e.g. QuakeCharacterController) has to
	// re-sequence it rather than call a shared helper -- but it still
	// needs read/write access to reuse it instead of reimplementing its
	// own parallel copy.
	float currentCameraHeight = 0.0f;
	float targetCameraHeight = 0.0f;

	void UpdateGroundCheck(bool& hitsGround, float& calculatedCharacterHeight, bool& canStand, vec3& walkableNormal, vec3& notWalkableNormal);

	bool CheckGroundAt(vec3 location, float radius, float& height, bool& canStand, vec3& normal, const Body** hitBody, Entity** hitEntity = nullptr);

	// Detects characters we are standing on, manages AddIgnorePair/RemoveIgnorePair,
	// and applies horizontal separation impulses to both bodies.
	void UpdateCharacterStacking(float deltaTime);

protected:

	Entity* owner = nullptr;
	float standingHeight = 1.8f;

	// Saved before SetLadderMode(true) zeroes stepHeight; restored on exit.
	float savedStepHeight = 0.4f;

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
	std::unordered_map<BodyID, Delay> removeCollisionCooldown;

	bool CanStandUp();

};