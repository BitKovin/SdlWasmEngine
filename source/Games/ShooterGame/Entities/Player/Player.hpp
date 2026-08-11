#pragma once

#include <Entity.h>

#include <Input.h>

#include <MathHelper.hpp>

#include <Camera.h>

#include <Physics.h>

#include <DebugDraw.hpp>

#include <algorithm>   // for std::clamp
#include <cmath>       // for std::max
#include <functional>  // for std::function (inventory callbacks)

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

#include <ItemsDataBase.h>

#include <InteractionSystem/IInteractive.h>

#include <Entities/Pickup/Keys/KeyTypes.h>

#include <Entities/PointLight.h>

#include <Analytics/AnalyticsSystem.h>
#include <Helpers/StringHelper.h>

// Forward declaration for custom item logic
class Player;


// Inventory system structures
struct InventoryItem
{
	std::string itemID;              // Unique item identifier for item database lookup

	std::string uid;                 // Unique instance ID (for tracking specific instances of items, if needed)

	// Weapon data (used by MainWeapon, OffhandWeapon, DualWeapon types)
	WeaponSlotData mainWeaponData;   // Main weapon data (ammo, className, etc.)
	WeaponSlotData offhandWeaponData; // Offhand weapon data (for dual weapons or offhand items)

	int stackSize = 1;               // Number of items in stack (for stackable items)

	InventoryItem() = default;

	InventoryItem(const std::string& itemID, int stackSize = 1)
		: itemID(itemID), stackSize(stackSize)
	{}

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(InventoryItem, itemID, uid, mainWeaponData, offhandWeaponData, stackSize)

};

enum class WeaponSystemMode
{
	Slots,      // Traditional slot-based system (1-9 keys)
	Inventory   // Full inventory system with item management
};

// ── Movement state machine ────────────────────────────────────────────────────
// Single source of truth for the player's current locomotion mode.
// UpdateWalkMovement() dispatches to a per-state update function each frame.
// Transitions are always explicit: call Enter*/Exit* helpers, never set directly.
enum class MoveState
{
	Default,   // Ground walk, air, wall-jump — the "normal" movement bucket
	Sliding,   // Crouch-slide; shares setup code with Default via UpdateStateGroundAir
	Mantling,  // Ledge pull-up animation; blocks all other input
	OnLadder,  // Ladder climb; constant vertical speed, weapon hidden
};

class Player : public Entity
{

private:

	std::map<WeaponAmmoType, int> ammoCounts = { {WeaponAmmoType::None,0},
												{WeaponAmmoType::PistolBullets,16},
												{WeaponAmmoType::ShotgunShells,4},
												{WeaponAmmoType::CannonBullets,0} };
	std::map<WeaponAmmoType, int> ammoLimits = { {WeaponAmmoType::None,0},
												{WeaponAmmoType::PistolBullets,96},
												{WeaponAmmoType::ShotgunShells,48},
												{WeaponAmmoType::CannonBullets,12} };


	int numTouchingLadders = 0;

	PointLight* playerLight = nullptr;

	float maxSpeed = 7.0f;
	float maxSpeedAir = 2;
	float acceleration = 90;
	float airAcceleration = 10;

	vec3 velocity = vec3(0);

	bool canRun = false;
	bool canDash = true;

	vec3 oldPos = vec3();



	bool freeFly = false;


	float bobProgress = 0;
	float bobSpeed = 1.1f;


	int lastSlot = -1;
	WeaponSlotData meleeWeapon;


	SkeletalMesh* bikeMesh = nullptr;

	SkeletalMesh* armsMesh = nullptr;

	bool on_bike = false;

	float cameraHeightOffset = 0;

	Delay stepDelay = Delay();

	float bike_progress = 0;

	vec3 stepForceWalkDirection = vec3();

	Delay afterStepDelay;

	Delay coyoteTime;



	bool teleported = false;

	Delay dashProgress;
	vec3 dashVector = vec3(0);
	bool wasDashing = false;

	std::shared_ptr<ObservationTarget> observationTarget;

	Body* hitbox = nullptr;

	int CurrentMaxRestrictionLevel = 0;
	int CurrentClearance = 0;

	// Inventory system
	WeaponSystemMode weaponSystemMode = WeaponSystemMode::Slots; // Default to inventory mode
	std::vector<InventoryItem> inventory;

	std::string lastInventoryUUID = "";      // Previously equipped inventory item UUID (for quick switch)
	bool pendingInventorySwitch = false;

	Delay mantleDelay;
	vec3 mantleStartPosition;
	vec3 mantleTargetPosition;
	// Ledge height range relative to the player's current feet position.
	static constexpr float MantleMinLedgeHeight = 0.3f;   // units above feet
	static constexpr float MantleMaxLedgeHeight = 2.5f;   // units above feet
	// How far in front of the player to search for a wall face.
	static constexpr float MantleForwardReach = 1.0f;
	// Minimum free vertical space above the ledge surface for the player to stand.
	static constexpr float MantleStandClearance = 0.15f;
	// Total seconds for the full pull-up + vault-over animation.
	static constexpr float MantleDuration = 1.050f;
	// Seconds before another TryMantle() call is allowed.
	static constexpr float MantleCooldown = 0.35f;
	// 0 → 1 over MantleDuration seconds, drives the two-phase easing curve.
	float mantleProgress = 0.0f;
	// Capsule-center the player is teleported to at mantle start
	vec3  mantleSnapPosition = vec3(0);

	vec3 weaponRunRotation = vec3(-8.9f, 30.0f, -9.21f);
	vec3 weaponSlideRotation = vec3(0, 0, -16);
	vec3 runRotatePoint = vec3(-0.05, -0.1, 0.45);// vec3(-0.1f, -0.290f, 0.45f);

	float slideInterp = 0;

	float WalkSpeed = 6.0f;// 4.5f;
	float CrouchSpeed = 2.5f;
	float RunSpeed = 7.5f;

	// ── Weapon suppression ────────────────────────────────────────────────────
	// True while CanHoldWeapon() == false.  Weapon objects are destroyed but
	// all UUID / slot state is preserved so RestoreWeapons() can rebuild them
	// correctly for both weapon-type and entity-type inventory items.
	bool weaponSuppressed = false;

	// Records which slots had live weapon objects at the moment of suppression.
	// Prevents RestoreWeapons() from recreating slots that were already empty.
	bool mainWasSuppressed = false;
	bool offhandWasSuppressed = false;

	// Returns false if any live weapon's CanChangeSlot() blocks removal this frame.
	// TrySuppressWeapons() polls this and is retried next frame if it returns false.
	bool CanSuppressWeapons() const;

	// Destroys live weapon objects in both slots, recording which were alive.
	// Returns false (and leaves everything untouched) when CanSuppressWeapons()
	// returns false — the caller should retry next frame.
	// Pass forceSuppress = true (mantle / death) to skip CanChangeSlot checks
	// and destroy weapons immediately regardless of their current state.
	bool TrySuppressWeapons(bool forceSuppress = false);

	// Recreates weapon objects from preserved UUID / slot state.
	// Handles Inventory and Slots modes, weapon-type items, and entity-type items.
	// Prefers desiredInventoryUUID over currentInventoryUUID for the main slot so
	// that a switch requested during suppression is not silently discarded.
	void RestoreWeapons();

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
		vel = Friction(vel, 40);

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

	void TryMantle();
	void StartMantle();
	void FinishMantle(bool isNaturalFinish = true);
	void UpdateMantle();

	void TryWallJump();

	void Jump()
	{

		if (dead)return;

		ANALYTICS_SEND_EVENT(
			"player_jump",
			std::unordered_map<std::string, std::string>{
				{"position", to_string(Position)},
				{ "velocity", to_string(controller.GetVelocity()) },
				{ "on_ground", to_string(controller.onGround) },
				{ "coyote_time_available", to_string(coyoteTime.Wait()) },
				{ "free_walljumps", to_string(freeWalljumps) }
		}
		);

		controller.UnCrouch();

		vec3 velocity = controller.GetVelocity();
		velocity.y = 8.0;
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

	ItemDbEntry GetItemData(const std::string& itemID);

	vec3 testStart;

	friend class InventoryMenu;

	IInteractive* currentInteractionObject = nullptr;
	bool startedInteracting = false;
	float interactionProgress = 0;

	friend class UseIndicator;

	// ── Movement state machine ────────────────────────────────────────────────
	MoveState moveState = MoveState::Default;

	// ── State predicates (read-only, derived from moveState) ─────────────────
	bool IsMantling()  const { return moveState == MoveState::Mantling; }
	bool IsSliding()   const { return moveState == MoveState::Sliding; }
	bool IsOnLadder()  const { return moveState == MoveState::OnLadder; }

	// ── Per-state update functions ────────────────────────────────────────────
	// UpdateWalkMovement() dispatches to one of these every frame.
	void UpdateStateGroundAir(vec2 input);  // Default + Sliding (shared setup)
	void UpdateStateLadder(vec2 input);     // OnLadder

	// ── Ladder helpers ────────────────────────────────────────────────────────
	// EnterLadder/ExitLadder are the only places that set OnLadder state.
	void EnterLadder(float inputY);
	void TryGrabLadderDeferred(float inputY);
	void ExitLadder();

	// Ladder tuning
	static constexpr float LadderClimbSpeed = 4.0f;   // units/sec up or down
	static constexpr float LadderLookDeadZone = 0.0f;   // degrees of pitch before input activates

	// ── Slide state ───────────────────────────────────────────────────────────
	vec3  slideDir = vec3(0);          // normalized horizontal slide direction
	Delay slideBoostCooldown;          // 2-second cooldown between slide boosts

	bool  wasOnGround = false;
	float airVerticalVelocity = 0.0f;  // vy captured while airborne, consumed on landing

	// Slide tuning constants
	static constexpr float SlideInitialBoost = 1.5f;
	static constexpr float SlideFriction = 4.0f;
	static constexpr float SlopeGravityScale = 1.5f;
	static constexpr float SlideSteerStrength = 0.4f;
	static constexpr float SlideBoostCooldownTime = 2.0f;
	static constexpr float SlideInputAlignment = 0.7f;
	static constexpr float SlideCancelAlignment = -0.6f;
	static constexpr float SlopeTriggerThreshold = 1.5f;
	static constexpr float LandingTransferScale = 0.5f;

	// ── Slope helpers ─────────────────────────────────────────────────────────
	void GetSlopeInfo(const vec3& groundNormal,
		vec3& outDownhillDir,
		float& outNetAccel) const;

	void StartSlide(const vec3& currentVelocity);
	void StopSlide();

	bool ShouldAutoSlide(const vec3& downhillDir, float netSlopeAccel, const vec3& wishDir) const;

	void UpdateSlide(vec2 input, const vec3& downhillDir, float netSlopeAccel);

	void Death();

public:


	PlayerHud Hud;

	PlayerBodyAnimator bodyAnimator = PlayerBodyAnimator(this);

	std::set<DoorKey> keysInventory;

	bool dead = false;
	Delay deathAnimDelay;

	Delay jumpDelay;

	int freeWalljumps = 1;
	float stamina = 3;
	bool disableStaminaRegenUntilGrounded = false;

	std::string desiredInventoryUUID = "";   // Item player wants to switch to UUID (for lazy switching)
	std::string currentInventoryUUID = "";   // Currently equipped item UUID from inventory

	std::string currentMainWeaponUUID = ""; // UUID of currently equipped main weapon (for inventory tracking)
	std::string currentOffhandWeaponUUID = ""; // UUID of currently equipped offhand weapon (

	vec3 cameraRotation = vec3(0);

	Weapon* currentWeapon = nullptr;

	Weapon* currentOffhandWeapon = nullptr;

	CharacterController controller;

	SkeletalMesh* bodyMesh = nullptr;

	std::string currentWeaponType = "";

	int currentSlot = 0;
	std::vector<WeaponSlotData> weaponSlots;

	std::vector<std::string> offhandWeapons = {""}; // "weapon_lefthand_empty" 
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
		armsMesh = new SkeletalMesh(this);
		Drawables.push_back(armsMesh);

		bodyMesh = new SkeletalMesh(this);
		bodyMesh->TwoSided = true;
		//bodyMesh->CastDetailShadows = false;
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
	
	void PostLoadStart() override;

	void Heal(float healthToAdd);

	// UpdateWalkMovement is now a thin dispatcher to per-state functions.
	// Add new locomotion modes by adding a MoveState value and an Update* method.
	void UpdateWalkMovement(vec2 input);
	void UpdateBikeMovement(vec2 input);

	void SwitchToSlot(int slot, bool forceChange = false);
	void SwitchToMeleeWeapon(bool forceChange = false);
	void AddWeapon(const WeaponSlotData& weaponData);
	void AddWeaponByName(const string& className);

	// Inventory system methods
	void SetWeaponSystemMode(WeaponSystemMode mode);
	WeaponSystemMode GetWeaponSystemMode() const { return weaponSystemMode; }

	// Inventory management
	std::string AddItemToInventory(const std::string& itemID, int stackSize = 1);

	bool RemoveItemFromInventory(const std::string& uuid);
	bool RemoveItemByID(const std::string& itemID);
	InventoryItem* GetInventoryItem(const std::string& uuid);
	InventoryItem* FindInventoryItemByUUID(const std::string& uuid);
	int FindInventoryItemByID(const std::string& itemID);
	const std::vector<InventoryItem>& GetInventory() const { return inventory; }

	int GetInventorySlotIdByUUID(const std::string& uuid);

	// Inventory weapon switching (with lazy switching support)
	void SwitchToInventoryItem(std::string uuid, bool forceChange = false);
	bool CanSwitchToInventoryItem(const std::string& uuid);
	void UpdateInventoryWeaponSwitch(); // Call in Update() to handle lazy switching


	void CreateWeapon(const string& className);
	void DestroyWeapon();

	int GetAmmoLimit(WeaponAmmoType type);
	int GetAmmo(WeaponAmmoType type);
	int SetAmmo(WeaponAmmoType type, int amount);
	int ConsumeAmmo(WeaponAmmoType type, int amount);
	int AddAmmo(WeaponAmmoType type, int amount);

	vec3 GetBobForMainWeapon();

	IInteractive* UpdateInteractionRaycast();
	void UpdateInteraction();

	void Destroy()
	{
		Entity::Destroy();

		Instance = nullptr;

	}

	bool HasStamina();
	void ConsumeStamina(float amount = 1.0f);
	void UpdateStamina();

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

	void OnLevelEnd();

	void StartedTouchLadder();
	void StoppedTouchLadder();

	// ── Weapon suppression ────────────────────────────────────────────────────

	// Returns false whenever the player must not hold physical weapon objects
	// (mantling, on ladder, on bike, dead, etc.).
	bool CanHoldWeapon() const;

	// Call once per Update(), before UpdateWeapon().
	void UpdateWeaponSuppression();

	// True while weapons are suppressed (objects destroyed, state preserved).
	bool IsWeaponSuppressed() const { return weaponSuppressed; }

	// Expose current locomotion state for external systems (HUD, animation, etc.)
	MoveState GetMoveState() const { return moveState; }

protected:

	void LoadAssets();

};