#pragma once

#include <Entity.h>

#include <Input.h>

#include <MathHelper.hpp>

#include <Camera.h>

#include <Physics.h>

#include <DebugDraw.hpp>

#include <algorithm>   // for std::clamp
#include <cmath>       // for std::max

#include <Navigation/Navigation.hpp>

#include <SkeletalMesh.hpp>

#include <imgui/imgui.h>

#include <Particle/ParticleSystem.hpp>

#include <Entities/SoundPlayer.h>
#include <SoundSystem/SoundManager.hpp>

#include <Character/CharacterController.h>
#include <Particle/GlobalParticleSystem.hpp>

#include <AiPerception/ObservationTarget.h>

#include "../../UI/Player/PlayerHud.hpp"

#include "Weapons/WeaponBase.h"

#include "PlayerBodyAnimator.h"

class Player : public Entity
{

private:

	float maxSpeed = 6.0f;
	float maxSpeedAir = 2;
	float acceleration = 90;
	float airAcceleration = 30;

	vec3 velocity = vec3(0);


	vec3 oldPos = vec3();

	Delay jumpDelay;

	bool freeFly = false;

	PlayerHud Hud;

	float bobProgress = 0;
	float bobSpeed = 1.1f;

	SoundPlayer* soundPlayer;

	int lastSlot = -1;
	WeaponSlotData meleeWeapon;


	SkeletalMesh* bikeMesh = nullptr;
	SkeletalMesh* bikeArmsMesh = nullptr;

	bool on_bike = false;

	float cameraHeightOffset = 0;

	Delay stepDelay = Delay();

	float bike_progress = 0;

	vec3 stepForceWalkDirection = vec3();

	Delay afterStepDelay;

	Delay coyoteTime;

	PlayerBodyAnimator bodyAnimator = PlayerBodyAnimator(this);

	bool teleported = false;

	Delay dashProgress;
	vec3 dashVector = vec3(0);
	bool wasDashing = false;

	std::shared_ptr<ObservationTarget> observationTarget;

	Body* hitbox = nullptr;

	int CurrentMaxRestrictionLevel = 0;
	int CurrentClearance = 0;



	vec3 weaponRunRotation = vec3(-8.9f, 30.0f, -9.21f);
	vec3 runRotatePoint = vec3(-0.1f,-0.290f,0.45f);

	float WalkSpeed = 4.5f;
	float RunSpeed = 7.5f;

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
		vec3 velocity = controller.GetVelocity();
		velocity.y = 7.5;
		controller.SetVelocity(velocity);

		jumpDelay.AddDelay(0.3);
	}

	bool CheckGroundAt(vec3 location)
	{

		if (jumpDelay.Wait())
			return false;

		if (afterStepDelay.Wait()) return true;

		auto result = Physics::LineTrace(location, location - vec3(0, 0.92, 0), BodyType::GroupCollisionTest, {  });

		return result.hasHit;

	}

	bool CanSwitchSlot(int slot);
	void SwitchWeapon(const WeaponSlotData& data);

	void SwitchWeaponOffhand(const string& classname);
	void DestroyWeaponOffhand();

	vec3 testStart;



public:

	vec3 cameraRotation = vec3(0);

	Weapon* currentWeapon = nullptr;

	Weapon* currentOffhandWeapon = nullptr;

	CharacterController controller;

	SkeletalMesh* bodyMesh = nullptr;

	int currentSlot = 0;
	std::vector<WeaponSlotData> weaponSlots;

	std::vector<std::string> offhandWeapons = {"weapon_lefthand_empty"};
	int offhandWeapon = 0;
	int desiredOffhandWeapon = 0;

	Delay violanceCrimeActiveDelay;

	float RunProgress = 0;

	bool ThirdPersonView = false;

	bool started = false;

	bool disableOffhandWeapon = false;

	Player()
	{

		bikeMesh = new SkeletalMesh(this);
		bikeMesh->Visible = false;
		Drawables.push_back(bikeMesh);
		bikeArmsMesh = new SkeletalMesh(this);
		bikeArmsMesh->Visible = false;
		Drawables.push_back(bikeArmsMesh);

		bodyMesh = new SkeletalMesh(this);
		bodyMesh->TwoSided = true;
		Drawables.push_back(bodyMesh);

		ClassName = "info_player_start";

		SaveGame = true;

		Tags = { "player" };

		Health = 100;

		LateUpdateWhenPaused = true;

		weaponSlots.resize(10);


	}
	~Player() { Logger::Log("player destructor"); }

	float Speed = 5;

	static Player* Instance;

	void FromData(EntityData data)
	{
		Entity::FromData(data);
		Rotation.y = data.GetPropertyFloat("angle") + 90;
	}

	void Start();

	void UpdateWalkMovement(vec2 input);
	void UpdateBikeMovement(vec2 input);

	void SwitchToSlot(int slot, bool forceChange = false);
	void SwitchToMeleeWeapon(bool forceChange = false);
	void AddWeapon(const WeaponSlotData& weaponData);
	void AddWeaponByName(const string& className);

	void CreateWeapon(const string& className);
	void DestroyWeapon();

	vec3 GetBobForMainWeapon();

	void Destroy()
	{
		Entity::Destroy();

		Instance = nullptr;

	}

	dtObstacleRef playerObstacle = 0;

	void UpdateWeapon();

	void UpdateDebugUI();

	bool OnGround();

	void PerformAttack();

	void TryStep(vec3 dir);

	void Update();
	void AsyncUpdate();
	void LateUpdate();

	void UpdateThirdPersonCamera();

	void UpdateBody();

	bool InThirdPerson();


	void OnDamage(float Damage, Entity* DamageCauser = nullptr, Entity* Weapon = nullptr);
	void OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon);

	void UpdateCurrentRestrictedArea();

	void Serialize(json& target);
	void Deserialize(json& source) override;

	void Teleport(vec3 target) override;

	void MoveTo(vec3 target);

	void StartBike();
	void StopBike();
	void ToggleBike();

protected:

	void LoadAssets();

};