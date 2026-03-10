#include "Player.hpp"

#include <Particle/GlobalParticleSystem.hpp>

#include <EngineMain.h>

#include <AiPerception/AiPerceptionSystem.h>

#include "Weapons/WeaponFirearm.h"

#include "RestrictedArea.h"

#include <SaveSystem/GameSaveSystem.h>
#include "../NpcSimulationManager.h"
#include <SpatialSound/SpatialSoundManager.h>

#include "../../UI/General/UiLocationTileDrop.h"

#include <Helpers/LightVisibilityHelper.h>
#include <UUID.hpp>

#include <PauseGameManager.hpp>

REGISTER_ENTITY(Player, "player")

Player* Player::Instance = nullptr;

string serializedPlayer = "";

void Player::Start()
{

	if (started) return;

	Entity::Start();

	started = true;

	Instance = this;

	observationTarget = AiPerceptionSystem::CreateTarget(Position, Id, { "player" });

	controller.Init(this, Position, 0.4f);
	controller.gravity = 24;
	oldPos = controller.GetPosition();

	ParticleSystem::PreloadSystemAssets("decal_blood");
	ParticleSystem::PreloadSystemAssets("hit_flesh");


	soundPlayer = new SoundPlayer();
	Level::Current->AddEntity(soundPlayer);
	soundPlayer->Sound = SoundManager::GetSoundFromPath("GameData/sounds/mew.wav");

	Hud.Init(this);



	hitbox = Physics::CreateCharacterBody(this, Position, 0.3f, 1.2f, 0.1f, BodyType::HitBox, BodyType::None);
	hitbox->SetMotionType(JPH::EMotionType::Kinematic);
	Physics::ExcludedDrawBodies.insert(hitbox);

	PreloadEntityType("weapon_pistol");
	PreloadEntityType("weapon_shotgun");
	PreloadEntityType("weapon_tommy");
	PreloadEntityType("weapon_sniper");
	PreloadEntityType("weapon_swords");
	PreloadEntityType("weapon_mpsd");

	PreloadEntityType("weapon_lefthand_empty");

	PreloadEntityType("weapon_cane");

	// Add weapons based on current weapon system mode
	if (weaponSystemMode == WeaponSystemMode::Inventory)
	{
		// Inventory mode - add weapons to inventory
		Weapon* tempWeapon;
		WeaponSlotData weaponData;
		std::string firstWeaponUUID;

		AddItemToInventory("weapon_pistol"); // Add pistol to inventory first so it's equipped by default
		AddItemToInventory("weapon_shotgun");
		AddItemToInventory("weapon_tommy");
		AddItemToInventory("weapon_sniper");
		AddItemToInventory("weapon_cane");
		AddItemToInventory("weapon_mpsd");

		/*
		// Add pistol
		tempWeapon = (Weapon*)LevelObjectFactory::instance().create("weapon_pistol");
		weaponData = tempWeapon->GetDefaultData();
		delete tempWeapon;
		firstWeaponUUID = AddItemToInventory("weapon_pistol", weaponData);

		// Add shotgun
		tempWeapon = (Weapon*)LevelObjectFactory::instance().create("weapon_shotgun");
		weaponData = tempWeapon->GetDefaultData();
		delete tempWeapon;
		AddItemToInventory("weapon_shotgun", weaponData);


		// Add tommy gun
		tempWeapon = (Weapon*)LevelObjectFactory::instance().create("weapon_tommy");
		weaponData = tempWeapon->GetDefaultData();
		delete tempWeapon;
		AddItemToInventory("weapon_tommy", weaponData);

		// Add sniper
		tempWeapon = (Weapon*)LevelObjectFactory::instance().create("weapon_sniper");
		weaponData = tempWeapon->GetDefaultData();
		delete tempWeapon;
		AddItemToInventory("weapon_sniper", weaponData);

		// Add offhand weapons to inventory
		WeaponSlotData caneData;
		tempWeapon = (Weapon*)LevelObjectFactory::instance().create("weapon_cane");
		caneData = tempWeapon->GetDefaultData();
		delete tempWeapon;
		AddItemToInventory("weapon_cane", InventoryItemType::OffhandWeapon, caneData);

		// Add dual weapon example (pistol + empty offhand for now)
		WeaponSlotData pistolData, emptyOffhand;
		tempWeapon = (Weapon*)LevelObjectFactory::instance().create("weapon_pistol");
		pistolData = tempWeapon->GetDefaultData();
		delete tempWeapon;

		tempWeapon = (Weapon*)LevelObjectFactory::instance().create("weapon_lefthand_empty");
		emptyOffhand = tempWeapon->GetDefaultData();
		delete tempWeapon;
		AddItemToInventory("pistol_dual", pistolData, emptyOffhand);
		*/

		// Equip first weapon
		if (!firstWeaponUUID.empty())
		{
			SwitchToInventoryItem(firstWeaponUUID, true);
		}
	}
	else
	{
		// Slots mode - use original system
		AddWeaponByName("weapon_pistol");
		AddWeaponByName("weapon_shotgun");
		AddWeaponByName("weapon_mpsd");
		AddWeaponByName("weapon_sniper");

		// Offhand weapons for slots mode
		offhandWeapons.push_back("weapon_cane");
		desiredOffhandWeapon = 2;
	}

	cameraRotation.y = Rotation.y;


	//Spawn("TestSpatialSoundPlayer")->Start();

}

void Player::UpdateWalkMovement(vec2 input)
{

	vec3 right = MathHelper::GetRightVector(Camera::rotation);

	vec3 forward = MathHelper::GetForwardVector(vec3(0, Camera::rotation.y, 0));

	if (freeFly)
		forward = Camera::Forward();

	vec3 movement = input.x * right + input.y * forward;

	//if (stepDelay.Wait())
		//movement = stepForceWalkDirection;




	velocity = controller.GetVelocity();


	if (OnGround())
	{

		bobProgress += length(MathHelper::XZ(velocity)) * Time::DeltaTime;

		/*
		TryStep(movement * 0.8f);

		TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), 5));
		TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), -5));

		TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), 10));
		TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), -10));

		TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), 20));
		TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), -20));

		TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), 35));
		TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), -35));
		*/
	}

	if (OnGround())
	{
		velocity = UpdateGroundVelocity(movement, velocity);
	}
	else
	{
		velocity = UpdateAirVelocity(movement, velocity);
	}


	velocity.y = controller.GetVelocity().y;

	if (freeFly)
		velocity = movement * 20.0f;

	controller.SetVelocity(velocity);

	if (Input::GetAction("crouch")->Pressed())
	{
		if (controller.isCrouched)
		{
			controller.UnCrouch();
		}
		else
		{
			controller.Crouch();
		}

	}



	if (Input::GetAction("jump")->PressedBuffered())
	{

		if (dashProgress.Wait() && HasStamina() && OnGround()) // dash jump
		{
			wasDashing = false;
			dashProgress.AddDelay(-1);
			ConsumeStamina();
			controller.SetVelocity(dashVector);
			Jump();
		}
		else
		{
			if (OnGround())
			{
				Jump();
			}
			else
			{
				TryWallJump();

				velocity = controller.GetVelocity();

				dashVector = normalize(dashVector) * 20.0f;

				controller.SetVelocity(velocity);

			}
		}

	}
}

void Player::UpdateBikeMovement(vec2 input)
{
	// Bike movement parameters
	const float maxSpeed = 15.0f;
	const float acceleration = 15.0f;
	const float lateralFriction = 1.5f;
	const float deltaTime = Time::DeltaTimeF; // Implement time handling

	vec3 moveRot = vec3(0, Camera::rotation.y - (input.x * 45.0 * 0), 0);

	// Get bike's forward direction (based on camera yaw)
	vec3 forward = MathHelper::GetForwardVector(moveRot);
	vec3 right = MathHelper::GetRightVector(moveRot);



	// Always move forward (override input)
	input = vec2(0, 1);
	vec3 movementDirection = input.x * right + input.y * forward;


	velocity = controller.GetVelocity();

	// Separate vertical and horizontal components
	float verticalVelocity = velocity.y;
	vec3 horizontalVelocity = vec3(velocity.x, 0.0f, velocity.z);

	// Calculate forward speed and lateral velocity
	float currentForwardSpeed = glm::dot(horizontalVelocity, forward);
	vec3 lateralVelocity = horizontalVelocity - (forward * currentForwardSpeed);

	// Apply forward acceleration with speed cap
	currentForwardSpeed = glm::min(currentForwardSpeed + acceleration * deltaTime, maxSpeed);

	// Apply lateral friction (drift effect)
	lateralVelocity *= glm::max(1.0f - lateralFriction * deltaTime, 0.0f);

	// Combine new velocity components
	vec3 newHorizontalVelocity = (forward * currentForwardSpeed) + lateralVelocity;
	vec3 newVelocity = vec3(newHorizontalVelocity.x, verticalVelocity, newHorizontalVelocity.z);

	// Update physics body velocity
	controller.SetVelocity(newVelocity);

	bikeMesh->Rotation.z = -dot(velocity, right) * 2.5f;
	bikeMesh->Rotation.y -= bikeMesh->Rotation.z * 0.2f;


	AnimationPose pose = bikeMesh->GetAnimationPose();

	MathHelper::Transform frontRot = pose.GetBoneTransform("pelvis");
	MathHelper::Transform wheelRot = pose.GetBoneTransform("wheel_front");
	MathHelper::Transform rightArm = pose.GetBoneTransform("upperarm_r");

	frontRot.Rotation -= vec3(0, bikeMesh->Rotation.z * 1.0, 0);
	wheelRot.Rotation += vec3(Time::GameTime * 1000.0f, 0, 0);

	if (currentWeapon)
	{
		rightArm.Scale = vec3(0);
	}

	pose.SetBoneTransformEuler("pelvis", frontRot);
	pose.SetBoneTransformEuler("wheel_front", wheelRot);
	pose.SetBoneTransformEuler("upperarm_r", rightArm);

	bikeMesh->PasteAnimationPose(pose);
	bikeArmsMesh->PasteAnimationPose(bikeMesh->GetAnimationPose());
	bikeArmsMesh->Rotation = bikeMesh->Rotation;

	if (Input::GetAction("jump")->Holding())
	{
		if (OnGround())
		{
			Jump();
		}
		else
		{
			TryWallJump();
		}
	}



}

void Player::TryWallJump()
{
	if (jumpDelay.Wait()) return;


	auto hit = Physics::SphereTrace(Position, Position + vec3(0, -0.1f, 0), 0.6f, BodyType::GroupCollisionTest & ~BodyType::CharacterCapsule, {}, { this });

	if (hit.hasHit)
	{

		if (HasStamina() == false) return;

		ConsumeStamina();

		vec3 wallNormal = hit.normal;

		vec3 velocity = controller.GetVelocity();

		// Decompose velocity into parallel and perpendicular components relative to the wall
		float perpMagnitude = glm::dot(velocity, wallNormal);
		vec3 perpComponent = wallNormal * perpMagnitude;   // into/away from wall
		vec3 paraComponent = velocity - perpComponent;     // sliding along wall

		// Keep all parallel (tangent) velocity, set perpendicular to a fixed launch speed
		const float wallJumpOutSpeed = 5.0f;
		const float wallJumpUpSpeed = 9.5f;

		vec3 newVelocity = paraComponent;                        // preserve lateral momentum
		newVelocity += wallNormal * wallJumpOutSpeed;        // fixed push off wall
		newVelocity.y = wallJumpUpSpeed;                      // override vertical

		controller.SetVelocity(newVelocity);

		jumpDelay.AddDelay(0.3f);
	}
}

bool Player::CanSwitchSlot(int slot)
{
	if (!currentWeapon) return true;
	if (slot == currentSlot) return false;
	return currentWeapon->CanChangeSlot();
}

void Player::SwitchWeapon(const WeaponSlotData& data)
{
	DestroyWeapon();

	//UiLocationTileDrop::PlayTitleCard(data.className);

	if (!data.className.empty())
	{
		currentWeapon = (Weapon*)Spawn(data.className);
		currentWeapon->owner = this;
		if (Level::Current->IsEntityTypeLoaded(data.className))
		{
			currentWeapon->LoadAssetsIfNeeded();
		}
		currentWeapon->Start();
		currentWeapon->SetData(data);
		//UpdateBody();
	}
}

void Player::SwitchWeaponOffhand(const string& classname)
{

	if (currentOffhandWeapon)
		if (classname == currentOffhandWeapon->ClassName) return;

	DestroyWeaponOffhand();

	if (!classname.empty())
	{
		currentOffhandWeapon = (Weapon*)Spawn(classname);
		currentOffhandWeapon->owner = this;
		currentOffhandWeapon->Start();
		currentOffhandWeapon->LoadAssetsIfNeeded();
	}

}

void Player::DestroyWeaponOffhand()
{

	if (currentOffhandWeapon != nullptr)
	{

		currentOffhandWeapon->Destroy();
		currentOffhandWeapon = nullptr;

	}

	currentOffhandWeaponUUID = "";


}

ItemDbEntry Player::GetItemData(const std::string& itemID)
{
	return ItemsDataBase::GetItemData(itemID);
}

void Player::SwitchToSlot(int slot, bool forceChange)
{
	if (!forceChange && !CanSwitchSlot(slot))
	{
		if (slot != currentSlot) // currentWeapon->IsMelee() && 
		{
			lastSlot = currentSlot;
			currentSlot = slot;
		}
		return;
	}

	if (slot < 0 || slot >= weaponSlots.size()) return;
	if (weaponSlots[slot].className.empty()) return;

	lastSlot = currentSlot;
	currentSlot = slot;
	SwitchWeapon(weaponSlots[slot]);
}

void Player::SwitchToMeleeWeapon(bool forceChange)
{
	if (!forceChange && currentWeapon && currentWeapon->IsMelee())
		return;

	if (!forceChange && currentWeapon && !currentWeapon->CanChangeSlot())
		return;

	if (!meleeWeapon.className.empty())
	{
		currentSlot = -1;
		SwitchWeapon(meleeWeapon);
	}
}

void Player::AddWeapon(const WeaponSlotData& weaponData)
{
	int slot = weaponData.slot;

	if (slot < 0 || slot >= weaponSlots.size()) return;

	if (weaponSlots[slot].className.empty() ||
		weaponSlots[slot].priority < weaponData.priority)
	{
		weaponSlots[slot] = weaponData;

		if (currentSlot == slot)
		{
			SwitchToSlot(slot, true);
		}
	}
}

void Player::AddWeaponByName(const string& className)
{

	Weapon* weap = (Weapon*)LevelObjectFactory::instance().create(className);

	AddWeapon(weap->GetDefaultData());

	delete(weap);

}

void Player::CreateWeapon(const string& className)
{

	Weapon* weap = (Weapon*)Spawn(className);

	weap->Start();
	weap->LoadAssetsIfNeeded();

	currentWeapon = weap;

}

void Player::DestroyWeapon()
{
	if (currentWeapon)
	{
		currentWeapon->Destroy();
		currentWeapon = nullptr;
	}
	currentMainWeaponUUID = "";
}

// ============================================================================
// INVENTORY SYSTEM IMPLEMENTATION
// ============================================================================

void Player::SetWeaponSystemMode(WeaponSystemMode mode)
{
	weaponSystemMode = mode;

	// When switching modes, clear the current weapon to avoid conflicts
	if (mode == WeaponSystemMode::Inventory)
	{
		currentSlot = -1;
		// Optionally preserve current weapon in inventory
	}
	else // Switching to Slots mode
	{
		currentInventoryUUID = "";
		desiredInventoryUUID = "";
		pendingInventorySwitch = false;
	}
}

// Add main weapon only (backwards compatible)
std::string Player::AddItemToInventory(const std::string& itemID, int stackSize)
{
	// Check if item already exists and can be stacked
	int existingIndex = FindInventoryItemByID(itemID);

	if (existingIndex >= 0)
	{
		// Stack with existing item if possible
		inventory[existingIndex].stackSize += stackSize;
		return inventory[existingIndex].uid;
	}

	// Add as new item
	InventoryItem newItem(itemID, stackSize);
	newItem.uid = UUID::generate_uuid();

	auto itemData = GetItemData(itemID);

	if (itemData.weaponClassName.empty() == false)
	{
		auto tempWeapon = (Weapon*)LevelObjectFactory::instance().create(itemData.weaponClassName);
		newItem.mainWeaponData = tempWeapon->GetDefaultData();
		newItem.mainWeaponData.inventoryUUID = newItem.uid; // Link weapon data to inventory item
		delete tempWeapon;
	}

	if (itemData.weaponOffhandClassName.empty() == false)
	{
		auto tempWeapon = (Weapon*)LevelObjectFactory::instance().create(itemData.weaponOffhandClassName);
		newItem.offhandWeaponData = tempWeapon->GetDefaultData();
		newItem.offhandWeaponData.inventoryUUID = newItem.uid; // Link weapon data to inventory item
		delete tempWeapon;
	}

	inventory.push_back(newItem);
	return newItem.uid;
}


bool Player::RemoveItemFromInventory(const std::string& uuid)
{
	// Find the item by UUID
	auto it = std::find_if(inventory.begin(), inventory.end(),
		[&uuid](const InventoryItem& item) { return item.uid == uuid; });

	if (it == inventory.end())
		return false;

	// If this is the currently equipped item, unequip it first
	if (uuid == currentInventoryUUID)
	{
		DestroyWeapon();
		currentInventoryUUID = "";
	}

	// Update pending switch if it was pointing to this item
	if (uuid == desiredInventoryUUID)
	{
		desiredInventoryUUID = "";
		pendingInventorySwitch = false;
	}

	// Remove the item
	inventory.erase(it);
	return true;
}

bool Player::RemoveItemByID(const std::string& itemID)
{
	int index = FindInventoryItemByID(itemID);
	if (index >= 0)
	{
		return RemoveItemFromInventory(inventory[index].uid);
	}
	return false;
}

InventoryItem* Player::GetInventoryItem(const std::string& uuid)
{
	return FindInventoryItemByUUID(uuid);
}

InventoryItem* Player::FindInventoryItemByUUID(const std::string& uuid)
{
	auto it = std::find_if(inventory.begin(), inventory.end(),
		[&uuid](const InventoryItem& item) { return item.uid == uuid; });

	if (it != inventory.end())
		return &(*it);
	return nullptr;
}

int Player::FindInventoryItemByID(const std::string& itemID)
{
	for (int i = 0; i < inventory.size(); i++)
	{
		if (inventory[i].itemID == itemID)
			return i;
	}
	return -1;
}

int Player::GetInventorySlotIdByUUID(const std::string& uuid)
{

	if (uuid.empty())
		return -1;

	int index = 0;

	for (auto& item : inventory)
	{

		if (item.uid == uuid) return index;

		index++;

	}

	return -1;
}

bool Player::CanSwitchToInventoryItem(const std::string& uuid)
{
	if (uuid.empty())
		return false;

	// Check if item exists
	InventoryItem* item = FindInventoryItemByUUID(uuid);
	if (!item)
		return false;

	//if (uuid == currentInventoryUUID)
	//	return false;

	if (!currentWeapon)
		return true;

	return currentWeapon->CanChangeSlot();
}

void Player::SwitchToInventoryItem(std::string uuid, bool forceChange)
{

	Logger::Log("Attempting to switch to inventory (UUID: " + uuid + ")");


	// Validate UUID and check if item exists
	if (uuid.empty())
		return;

	InventoryItem* itemPtr = FindInventoryItemByUUID(uuid);
	if (!itemPtr)
		return;


	// Check if we can switch
	if (!forceChange && !CanSwitchToInventoryItem(uuid))
	{
		// Set up lazy switching - wait for weapon to allow change
		desiredInventoryUUID = uuid;
		pendingInventorySwitch = true;
		return;
	}

	// Clear any pending switch
	pendingInventorySwitch = false;
	desiredInventoryUUID = "";

	auto newItemData = GetItemData(itemPtr->itemID);

	InventoryItem* currentItem = FindInventoryItemByUUID(currentInventoryUUID);
	auto itemData = ItemDbEntry();

	if (currentItem)
	{
		itemData = GetItemData(currentItem->itemID);
	}

	if (itemData.offhandCompatible == false && newItemData.itemType == InventoryItemType::OffhandWeapon)
	{
		DestroyWeapon();
	}

	// Save current weapon state back to inventory before switching
	if (currentWeapon && !currentInventoryUUID.empty())
	{

		if (currentItem)
		{

			// Save based on item type
			if (itemData.itemType == InventoryItemType::MainWeapon ||
				itemData.itemType == InventoryItemType::DualWeapon)
			{
				currentItem->mainWeaponData = currentWeapon->Data;
			}

			if (itemData.itemType == InventoryItemType::DualWeapon && currentOffhandWeapon)
			{
				currentItem->offhandWeaponData = currentOffhandWeapon->Data;
			}
			else if (itemData.itemType == InventoryItemType::OffhandWeapon && currentOffhandWeapon)
			{
				currentItem->offhandWeaponData = currentOffhandWeapon->Data;
			}
		}
	}



	if (currentMainWeaponUUID == uuid || (currentOffhandWeaponUUID == uuid))
	{

		auto itemData = newItemData;

		// Handle different item types
		switch (itemData.itemType)
		{
		case InventoryItemType::MainWeapon:
		{
			// Equip main weapon only
			DestroyWeapon();
			currentInventoryUUID = "";
			return;
			break;
		}

		case InventoryItemType::OffhandWeapon:
		{
			DestroyWeaponOffhand();
			currentInventoryUUID = "";
			return;
			break;
		}

		case InventoryItemType::DualWeapon:
		{
			DestroyWeapon();
			DestroyWeaponOffhand();
			currentInventoryUUID = "";
			return;
		}

		case InventoryItemType::CustomLogic:
		{

		}
		}
	}

	// Track last inventory UUID for quick switching
	if (uuid != currentInventoryUUID && !currentInventoryUUID.empty())
	{
		lastInventoryUUID = currentInventoryUUID;
	}

	// Update current inventory UUID
	currentInventoryUUID = uuid;

	currentItem = FindInventoryItemByUUID(currentInventoryUUID);
	itemData = ItemDbEntry();

	if (currentItem != nullptr)
	{
		itemData = GetItemData(currentItem->itemID);


		// Handle different item types
		switch (itemData.itemType)
		{
			case InventoryItemType::MainWeapon:
			{

				if(itemData.offhandCompatible == false)
				{
					// If new main weapon is not compatible with offhand, destroy current offhand weapon
					DestroyWeaponOffhand();
				}

				// Equip main weapon only
				SwitchWeapon(currentItem->mainWeaponData);
				currentMainWeaponUUID = uuid;
				break;
			}

			case InventoryItemType::OffhandWeapon:
			{
				// Equip offhand weapon only
				//DestroyWeapon(); // Clear main weapon
				if (!currentItem->offhandWeaponData.className.empty())
				{
					SwitchWeaponOffhand(currentItem->offhandWeaponData.className);
					if (currentOffhandWeapon)
					{
						currentOffhandWeapon->SetData(currentItem->offhandWeaponData);
					}
				}

				currentOffhandWeaponUUID = uuid;

				break;
			}

			case InventoryItemType::DualWeapon:
			{

				// Equip both main and offhand weapons
				SwitchWeapon(currentItem->mainWeaponData);
				if (!currentItem->offhandWeaponData.className.empty())
				{
					SwitchWeaponOffhand(currentItem->offhandWeaponData.className);
					if (currentOffhandWeapon)
					{
						currentOffhandWeapon->SetData(currentItem->offhandWeaponData);
					}
				}

				currentMainWeaponUUID = uuid;
				currentOffhandWeaponUUID = uuid;

				break;
			}

			case InventoryItemType::CustomLogic:
			{

				if (itemData.interactionEntityClassname.empty() == false)
				{
					Spawn(itemData.interactionEntityClassname);

					if (itemData.destroyOnUse)
					{
						// Decrease stack size and remove if depleted
						currentItem->stackSize--;
						if (currentItem->stackSize <= 0)
						{
							RemoveItemFromInventory(uuid);
						}
					}

				}

				break;
			}
		}
	}
}

void Player::UpdateInventoryWeaponSwitch()
{
	// Only process in inventory mode
	if (weaponSystemMode != WeaponSystemMode::Inventory)
		return;

	// Check if there's a pending switch
	if (pendingInventorySwitch && !desiredInventoryUUID.empty())
	{
		// Try to switch again
		if (CanSwitchToInventoryItem(desiredInventoryUUID))
		{
			SwitchToInventoryItem(desiredInventoryUUID, true);
		}
	}


	// Update current weapon data in inventory based on item type
	if (!currentInventoryUUID.empty())
	{
		InventoryItem* currentItem = FindInventoryItemByUUID(currentInventoryUUID);

		auto itemData = GetItemData(currentItem->itemID);

		if (currentItem)
		{
			// Update main weapon data
			if (currentWeapon && (itemData.itemType == InventoryItemType::MainWeapon ||
				itemData.itemType == InventoryItemType::DualWeapon))
			{
				currentItem->mainWeaponData = currentWeapon->Data;
			}

			// Update offhand weapon data
			if (currentOffhandWeapon && (itemData.itemType == InventoryItemType::OffhandWeapon ||
				itemData.itemType == InventoryItemType::DualWeapon))
			{
				currentItem->offhandWeaponData = currentOffhandWeapon->Data;
			}
		}
	}
}

// ============================================================================
// END INVENTORY SYSTEM
// ============================================================================

vec3 Player::GetBobForMainWeapon()
{

	vec3 bobT = vec3(0);

	bobT.y = (float)(sin(bobProgress * bobSpeed * 2) + 0.2f) * -0.15f;
	bobT.x = (float)((sin(bobProgress * bobSpeed * 1)) - 0.15f) * 0.3f;

	return bobT * 0.02f;
}

IInteractive* Player::UpdateInteractionRaycast()
{

	auto hit = Physics::LineTrace(Camera::position, Camera::position + Camera::Forward() * 2.0f, BodyType::GroupHitTest, {}, { this, currentWeapon, currentOffhandWeapon });

	if (hit.hasHit == false) return nullptr;

	auto interactive = dynamic_cast<IInteractive*>(hit.entity);

	return interactive;

}

void Player::UpdateInteraction()
{

	IInteractive* newInteractive = UpdateInteractionRaycast();

	if (newInteractive != currentInteractionObject)
	{
		if (currentInteractionObject)
		{
			currentInteractionObject->InterruptedSecondaryInteractionHold(interactionProgress);
		}

		interactionProgress = 0;
		currentInteractionObject = newInteractive;

		startedInteracting = false;
	}

	if (!currentInteractionObject)
	{

		startedInteracting = false;
		interactionProgress = 0;

		return;
	}

	if (Input::GetAction("interact")->Pressed())
	{
		startedInteracting = true;
	}

	if (startedInteracting == false) return;

	if (currentInteractionObject->HasSecondaryInteraction() == false)
	{

		if (Input::GetAction("interact")->Pressed())
		{

			if (currentInteractionObject->CanBeInteracted())
			{
				currentInteractionObject->Interact(this);
				startedInteracting = false;
			}

		}

		return;
	}

	if (Input::GetAction("interact")->Released())
	{
		if (interactionProgress > 0)
		{
			currentInteractionObject->InterruptedSecondaryInteractionHold(interactionProgress);
			interactionProgress = 0;
		}
		else
		{
			if (Input::GetAction("interact")->GetHoldTime() < 0.3f || true)
			{
				if (currentInteractionObject->CanBeInteracted())
				{
					currentInteractionObject->Interact(this);
				}
			}
		}
		startedInteracting = false;
	}
	else if (Input::GetAction("interact")->Holding())
	{
		if (Input::GetAction("interact")->GetHoldTime() > 0.2f)
		{
			if (currentInteractionObject->HasSecondaryInteraction() && currentInteractionObject->CanBeInteractedSecondary())
			{
				interactionProgress += Time::DeltaTimeF / currentInteractionObject->GetSecondaryInteractionHoldTime();

				currentInteractionObject->PerformingSecondaryInteractionHold(interactionProgress);

				if (interactionProgress >= 1.0f)
				{
					startedInteracting = false;

					currentInteractionObject->InteractSecondary(this);

					interactionProgress = 0;
				}
			}
		}
	}

}

bool Player::HasStamina()
{
	return stamina >= 0.99;
}

void Player::ConsumeStamina(float amount)
{
	stamina = std::max(stamina - amount, 0.0f);
	disableStaminaRegenUntilGrounded = true;
}

void Player::UpdateStamina()
{

	if (dashProgress.Wait()) return;

	if (OnGround())
	{
		disableStaminaRegenUntilGrounded = false;
	}

	if (disableStaminaRegenUntilGrounded) return;

	stamina = std::min(stamina + Time::DeltaTimeF * 1.0f, 4.0f);

}

void Player::UpdateWeapon()
{

	vec3 bob = GetBobForMainWeapon();

	vec3 forwardOffset = Camera::Forward() * Camera::rotation.x * 0.01f;

	observationTarget->tags = { "player" };

	if (violanceCrimeActiveDelay.Wait())
	{
		observationTarget->tags.insert("violentCrime");
	}

	auto firearm = dynamic_cast<WeaponFirearm*>(currentWeapon);

	if (firearm != nullptr)
	{

		if (Input::GetAction("test")->Pressed())
		{
			if (firearm)
			{
				firearm->SetAkimbo(!firearm->akimbo);
			}

		}

		disableOffhandWeapon = firearm->akimbo 
			|| (currentWeapon && currentWeapon->SupportsOffhandWeapon == false);

		vec3 relativeWeaponPos = vec3();

		vec3 currentWeaponRunRotation = lerp(vec3(), weaponRunRotation, RunProgress);

		vec3 rotatedWeaponPos = MathHelper::RotateAroundPoint(relativeWeaponPos, runRotatePoint, currentWeaponRunRotation);

		glm::quat qCurrent = MathHelper::GetRotationQuaternion(lerp(cameraRotation, Camera::rotation, 0.75f));
		glm::quat qAdd = MathHelper::GetRotationQuaternion(currentWeaponRunRotation);

		glm::quat qResult = qCurrent * qAdd;

		rotatedWeaponPos -= mix(vec3(), vec3(-0.05f, 0.02, 0.05), RunProgress);

		vec3 scaledBob = bob * mix(vec3(1), vec3(2.5, 2.2f, 2.2f), RunProgress);

		currentWeapon->HideWeapon = (currentOffhandWeapon != nullptr) ? 1.0f : bike_progress;
		currentWeapon->Position = MathHelper::TransformVector(rotatedWeaponPos, Camera::GetMatrix()) + MathHelper::TransformVector(scaledBob, Camera::GetRotationMatrix()) * currentWeapon->bobScale;
		currentWeapon->Rotation = MathHelper::ToYawPitchRoll(qResult);// +vec3(40.0f, 30.0f, 30.0f) * bike_progress;

		if (currentWeapon->Illegal)
		{
			observationTarget->tags.insert("illegal_weapon");
		}

	}

	if (currentOffhandWeapon != nullptr)
	{

		currentOffhandWeapon->Position = Camera::position + MathHelper::TransformVector(vec3(0, -bob.y + 0.001, bob.x) * 2.0f, Camera::GetRotationMatrix());
		currentOffhandWeapon->Rotation = lerp(cameraRotation, Camera::rotation, 0.3f);

	}

}

char* debug_level_name = new char[100];

void Player::UpdateDebugUI()
{

	auto draw = ImGui::GetForegroundDrawList();

	string fps = "fps: " + to_string((int)(1.0 / Time::DeltaTimeNoTimeScale));

	draw->AddText(NULL, 24.0f, ImVec2(10, 10), IM_COL32(255, 255, 255, 255), fps.c_str());

	if (EngineMain::MainInstance->Paused == false) return;

	ImGui::Begin("navigation");

	ImGui::Checkbox("draw nav mesh", &NavigationSystem::DebugDrawNavMeshEnabled);

	if (ImGui::Button("PlaceObstacle"))
	{
		NavigationSystem::RemoveObstacle(playerObstacle);
		playerObstacle = NavigationSystem::CreateObstacleBox(Position - vec3(1, 1, 1), Position + vec3(1, 1, 1));
	}

	if (ImGui::Button("place start location"))
	{
		testStart = Position;
		DebugDraw::Line(Position, Position - vec3(0, 1, 0), 2, 0.1);
	}

	if (ImGui::Button("calculate path to player"))
	{

	}

	ImGui::End();

	ImGui::Begin("weapon");
	ImGui::DragFloat3("weaponRotationPoint", &runRotatePoint.x, 0.01);
	ImGui::DragFloat3("weaponRotation", &weaponRunRotation.x, 0.01);
	ImGui::End();

	ImGui::Begin("graphic");
	ImGui::SliderInt("multisample count", &EngineMain::MainInstance->MainRenderer->MultiSampleCount, 0, 8);
	ImGui::SliderFloat("resolution scale", &EngineMain::MainInstance->MainRenderer->ResolutionScale, 0, 10);
	ImGui::End();

	ImGui::Begin("debug");

	ImGui::DragFloat("time scale", &Time::TimeScale, 0.01f, 0.f, 3);
	ImGui::Checkbox("fixed simulation tick rate", &Time::SimulationLikeFixedTimeStep);

	if (ImGui::Checkbox("fly", &freeFly))
	{
		if (freeFly)
		{
			controller.SetCollisionMask(BodyType::None);
			controller.SetCollisionMask(BodyType::None);
		}
		else
		{
			controller.SetCollisionMask(BodyType::GroupCollisionTest);
			controller.SetCollisionMask(BodyType::CharacterCapsule);
		}
	}

	ImGui::InputText("level name", debug_level_name, 100);
	ImGui::SameLine();
	if (ImGui::Button("load"))
	{
		Level::LoadLevelFromFile(string(debug_level_name));
	}

	ImGui::Checkbox("draw physics", &Physics::DebugDraw);

	if (ImGui::Button("spawn guard npc"))
	{
		Entity* entity = Spawn("npc_guard");
		entity->Position = Camera::position + Camera::Forward() * 2.0f;
		entity->Start();
	}

	if (ImGui::Button("spawn civilian npc"))
	{
		Entity* entity = Spawn("npc_civilian");
		entity->Position = Camera::position + Camera::Forward() * 2.0f;
		entity->Start();
	}

	if (ImGui::Button("test serialization"))
	{

		json jPlayer;
		Serialize(jPlayer);

		serializedPlayer = jPlayer.dump(4);

		Logger::Log(serializedPlayer);

	}

	if (ImGui::Button("test DEserialization"))
	{

		//json jPlayer = json::parse(serializedPlayer);
		//Deserialize(jPlayer);

		//Logger::Log(jPlayer.dump(4));

	}

	ImGui::End();


}

bool Player::OnGround()
{
	return (coyoteTime.Wait() || afterStepDelay.Wait()) && jumpDelay.Wait() == false;
}

void Player::PerformAttack()
{


}

void Player::TryStep(vec3 dir)
{
	return;
	/*
	if (stepDelay.Wait()) return;

	vec3 pos = Position + dir/1.3f;

	if (pos == vec3())
		return;

	auto hit = Physics::LineTrace(pos, (pos - vec3(0, 0.85f, 0)), Physics::GetCollisionMask(LeadBody), {LeadBody});

	if (hit.hasHit == false)
		return;

	DebugDraw::Line(hit.position, hit.position + hit.normal);
	if (hit.normal.y < 0.9)
		return;



	vec3 hitPoint = hit.position;

	if (hitPoint == vec3())
		return;

	if (Physics::LineTrace(hitPoint + vec3(0, 0.05, 0), Position - vec3(0, 0.87f, 0), Physics::GetCollisionMask(LeadBody), { LeadBody }).hasHit == false)
	{
		return;
	}



	if (hitPoint.y > Position.y - 1 + 0.8f)
		return;

	if (Physics::SphereTrace(hitPoint + vec3(0,1,0) * 0.33f, hitPoint + vec3(0, 1, 0), 0.3f, Physics::GetCollisionMask(LeadBody), { LeadBody }).hasHit)
		return;

	if (Physics::SphereTrace(Position, Position + normalize(dir) * 0.2f, 0.3f, Physics::GetCollisionMask(LeadBody), { LeadBody }).hasHit)
		return;

	if (distance(hitPoint, Position) > 1.4)
		return;

	hit = Physics::LineTrace(Position, mix(Position, hitPoint, 1.1f) + vec3(0,1,0) * 0.2f, Physics::GetCollisionMask(LeadBody), {LeadBody});

	if (hit.hasHit)
	{

		return;
	}


	vec3 lerpPose = mix(Position, hitPoint, 0.0f);

	lerpPose.y = hitPoint.y + 1;

	float newOffset = Position.y - lerpPose.y;

	cameraHeightOffset += newOffset;
	Position.y -= newOffset;

	controller.SetPosition(lerpPose);

	Physics::SetBodyPosition(LeadBody,lerpPose);
	//DebugDraw::Line(lerpPose - vec3(0, 0.9f, 0), lerpPose + vec3(0, 1, 0), 10, 0.1f);

	//stepForceWalkDirection = normalize(MathHelper::XZ(hitPoint - Position));

	stepDelay.AddDelay(0.05f);
	afterStepDelay.AddDelay(0.1f);
	*/
}

void Player::Update()
{

	if (EngineMain::MainInstance->SimulatingGameTicks) return;

	UpdateStamina();

	auto lightData = Level::Current->BspData.GetLightvolColorPoint(Position * MAP_SCALE, true);

	float lightLevel = LightVisibility::Compute(lightData);

	//printf("light level: %f \n", lightLevel);

	//Logger::Log("Voxel world memory: " + std::to_string(memMB) + " MB");


	//int value = SpatialSoundManager::GetVoxelValueAt(Camera::position);

	//Logger::Log("player sound voxel: " + to_string(value));

	//printf("%i \n",SkeletalMesh::skelMeshes);

	if (Input::GetAction("cameraView")->Pressed())
	{
		ThirdPersonView = !ThirdPersonView;
	}


	if (teleported == false && freeFly == false)
	{

		vec3 dif = controller.GetPosition() - oldPos;

		if (length(dif) > 0.25)
		{
			vec3 dir = normalize(controller.GetPosition() - oldPos);



			auto hit = Physics::LineTrace(oldPos, controller.GetPosition(), BodyType::World | BodyType::WorldSkybox);

			if (hit.hasHit)
			{
				controller.SetPosition(hit.position - dir * 0.5f);

			}

			vec3 offset = vec3(0, 0.1f, 0);

			hit = Physics::SphereTrace(oldPos + offset, controller.GetPosition() + offset, 0.3f, BodyType::World | BodyType::WorldSkybox);

			if (hit.hasHit)
			{
				controller.SetPosition(hit.shapePosition - offset);

			}
		}

	}
	teleported = false;
	oldPos = controller.GetPosition();

	Position = controller.GetSmoothPosition();


	if (controller.onGround)
	{
		coyoteTime.AddDelay(0.1f);
	}


	if (Input::LockCursor)
	{

		float fovScale = Camera::FOV / 75.0f;

		//fovScale = mix(fovScale, 1.0f, 0.5f);

		cameraRotation.y += Input::MouseDelta.x * fovScale;
		cameraRotation.x -= Input::MouseDelta.y * fovScale;

		vec2 touchMovement = Hud.ScreenControls->TouchArea->GetTouchMovement();

		touchMovement /= -5.0;

		cameraRotation.y += touchMovement.x * fovScale;
		cameraRotation.x -= touchMovement.y * fovScale;

		cameraRotation.x = glm::clamp(cameraRotation.x, -80.0f, 80.0f);

		if (on_bike)
		{
			cameraRotation.x = glm::clamp(cameraRotation.x, -50.0f, 59.0f);
		}

	}


	vec2 input = Input::GetLeftStickPosition();

	input += Hud.ScreenControls->Joystick->InputPosition;

	if (Input::GetAction("forward")->Holding())
		input += vec2(0, 1);

	if (Input::GetAction("backward")->Holding())
		input += vec2(0, -1);

	if (Input::GetAction("left")->Holding())
		input += vec2(-1, 0);

	if (Input::GetAction("right")->Holding())
		input += vec2(1, 0);

	if (length(input) > 1)
		input = normalize(input);

	if (canRun)
	{

		if (Input::GetAction("dash")->Holding() && input.y > 0.4f && OnGround())
		{
			RunProgress += Time::DeltaTimeF * 4.0f;
		}
		else
		{
			RunProgress -= Time::DeltaTimeF * 4.0f;
		}
	}

	RunProgress = std::clamp(RunProgress, 0.0f, 1.0f);

	maxSpeed = std::lerp(controller.isCrouched ? CrouchSpeed : WalkSpeed, RunSpeed, RunProgress);

	if (on_bike == false)
	{
		UpdateWalkMovement(input);
	}

	if (on_bike)
	{
		bike_progress += Time::DeltaTimeF * 3;
	}
	else
	{
		bike_progress -= Time::DeltaTimeF * 3;
	}

	bike_progress = glm::clamp(bike_progress, 0.0f, 1.0f);

	bikeMesh->Position = Position - vec3(0, 0.9f - 0.8f, 0);
	bikeMesh->Rotation = vec3(0, cameraRotation.y, 0);
	bikeMesh->Update();
	if (on_bike)
	{
		UpdateBikeMovement(input);
	}
	bikeArmsMesh->Rotation = bikeMesh->Rotation;
	bikeArmsMesh->Position = bikeMesh->Position;
	bikeArmsMesh->PasteAnimationPose(bikeMesh->GetAnimationPose());

	vec3 playerForward = MathHelper::GetForwardVector(vec3(0, cameraRotation.y, 0));

	cameraHeightOffset = mix(cameraHeightOffset, 0.0f, Time::DeltaTimeF * 5.0f);
	Camera::position = Position + vec3(0, 0.7, 0) - vec3(0, 0.25f, 0) * bike_progress + vec3(0, 1, 0) * cameraHeightOffset + playerForward * 0.1f;



	Camera::rotation = cameraRotation;

	vec3 right = MathHelper::GetRightVector(Camera::rotation);

	Camera::rotation.z = -dot(velocity, right) * mix(-0.2f, 0.3f, bike_progress);

	if (InThirdPerson() == false)
	{
		UpdateBody();
	}



	if (Input::GetAction("bike")->Holding() && OnGround())
	{
		StartBike();
	}
	else
	{
		StopBike();
	}

	bool dashEnded = false;

	if (wasDashing && dashProgress.Wait() == false)
	{
		dashEnded = true;
	}

	wasDashing = false;
	if (dashProgress.Wait())
	{
		controller.SetVelocity(dashVector);
		wasDashing = true;
	}
	else
	{

		if (dashEnded)
		{
			controller.SetVelocity(normalize(dashVector) * WalkSpeed);
		}

		if (Input::GetAction("dash")->Pressed() && canDash && HasStamina())
		{

			vec3 dashDir = right * input.x + playerForward * input.y;

			if (length(input) < 0.1)
			{
				dashDir = playerForward;
			}

			dashVector = dashDir * 15.0f;

			dashProgress.AddDelay(0.25f);

			ConsumeStamina();

		}
	}



	if (Input::GetAction("qSave")->Pressed())
	{
		GameSaveSystem::SaveGameToFile("quicksave");
	}

	if (Input::GetAction("qLoad")->Pressed())
	{
		GameSaveSystem::LoadGameFromFile("quicksave");
	}

	// Weapon switching logic based on current mode
	if (weaponSystemMode == WeaponSystemMode::Slots)
	{
		// Original slot-based weapon switching with lazy loading
		if (currentWeapon != nullptr)
		{
			if (currentWeapon->Data.slot != currentSlot)
			{
				if (currentWeapon->CanChangeSlot())
				{
					SwitchWeapon(weaponSlots[currentSlot]);
				}
			}
		}
		else
		{
			SwitchToSlot(currentSlot);
		}
	}
	else if (weaponSystemMode == WeaponSystemMode::Inventory)
	{
		// Inventory-based weapon switching with lazy loading
		UpdateInventoryWeaponSwitch();
	}



	// Offhand weapon management (Slots mode only)
	// In Inventory mode, offhand is managed through OffhandWeapon or DualWeapon items
	if (weaponSystemMode == WeaponSystemMode::Slots)
	{
		if (disableOffhandWeapon)
		{
			offhandWeapon = 0;
		}
		else
		{
			offhandWeapon = desiredOffhandWeapon;
		}

		if (offhandWeapons.empty() == false)
		{

			if (currentOffhandWeapon != nullptr)
			{
				if (currentOffhandWeapon->ClassName != offhandWeapons[offhandWeapon])
				{
					if (currentOffhandWeapon->CanChangeSlot())
					{
						SwitchWeaponOffhand(offhandWeapons[offhandWeapon]);
					}

				}
			}
			else
			{
				SwitchWeaponOffhand(offhandWeapons[offhandWeapon]);
			}
		}
	}

	if (Input::GetAction("slotMelee")->Pressed())
		SwitchToMeleeWeapon();

	// Adaptive input handling - works with both Slots and Inventory modes
	if (weaponSystemMode == WeaponSystemMode::Slots)
	{
		// Slot-based system (original behavior)
		if (Input::GetAction("slot1")->Pressed())
			SwitchToSlot(0);

		if (Input::GetAction("slot2")->Pressed())
			SwitchToSlot(1);

		if (Input::GetAction("slot3")->Pressed())
			SwitchToSlot(2);

		if (Input::GetAction("slot4")->Pressed())
			SwitchToSlot(3);

		if (Input::GetAction("slot5")->Pressed())
			SwitchToSlot(4);

		if (Input::GetAction("slot6")->Pressed())
			SwitchToSlot(5);

		if (Input::GetAction("lastSlot")->Pressed())
			SwitchToSlot(lastSlot);
	}
	else if (weaponSystemMode == WeaponSystemMode::Inventory)
	{
		// Inventory-based system (uses inventory indices)
		// Pressing the same weapon key twice will hide/unequip the weapon

		if (Input::GetAction("slot1")->Pressed())
		{
			if (inventory.size() > 0)
			{
				SwitchToInventoryItem(inventory[0].uid, false);
			}
		}

		if (Input::GetAction("slot2")->Pressed())
		{
			if (inventory.size() > 1)
			{
				SwitchToInventoryItem(inventory[1].uid, false);
			}
		}

		if (Input::GetAction("slot3")->Pressed())
		{
			if (inventory.size() > 2)
			{
				SwitchToInventoryItem(inventory[2].uid, false);
			}
		}

		if (Input::GetAction("slot4")->Pressed())
		{
			if (inventory.size() > 3)
			{
				SwitchToInventoryItem(inventory[3].uid, false);
			}
		}

		if (Input::GetAction("slot5")->Pressed())
		{
			if (inventory.size() > 4)
			{
				SwitchToInventoryItem(inventory[4].uid, false);
			}
		}

		if (Input::GetAction("slot6")->Pressed())
		{

			Logger::Log("Inventory size: " + to_string(inventory.size()));

			if (inventory.size() > 5)
			{
				SwitchToInventoryItem(inventory[5].uid, false);
			}
		}

		if (Input::GetAction("lastSlot")->Pressed())
		{
			SwitchToInventoryItem(lastInventoryUUID, false);
		}

		if (Input::GetAction("inventory")->Pressed())
		{
			Spawn("inventory_menu")->Start();
		}

	}

	UpdateInteraction();

}

void Player::AsyncUpdate()
{

	if (EngineMain::MainInstance->SimulatingGameTicks) return;

	if (InThirdPerson())
	{
		UpdateBody();
	}

	controller.Update(Time::DeltaTimeF);
	bodyAnimator.Update();

	UpdateCurrentRestrictedArea();

}

void Player::LateUpdate()
{

	if (EngineMain::MainInstance->Paused == false)
		if (Input::GetAction("pause")->Pressed())
		{
			PauseGameManager::SetGamePaused(!PauseGameManager::GetGamePaused());

		}

	UpdateWeapon();

	if (CurrentMaxRestrictionLevel > CurrentClearance)
	{
		observationTarget->tags.insert("trespassing");
	}

	Hud.Update();

}

void Player::UpdateThirdPersonCamera()
{

	vec3 forward = normalize(MathHelper::XZ(Camera::Forward()));

	Camera::position = Position;
	Camera::position -= forward * 0.1f;

	vec3 startPos = Position + vec3(0, 1, 0);

	vec3 targetCameraPos = Camera::position + vec3(0, 0.4f, 0);

	targetCameraPos += Camera::Forward() * -2.4f;
	targetCameraPos += Camera::Up() * 0.7f;
	//targetCameraPos += Camera::Right() * 0.05f;

	auto hit = Physics::SphereTrace(startPos, targetCameraPos, 0.3f, BodyType::GroupCollisionTest, {}, { this });
	if (hit.hasHit)
	{
		Camera::position = hit.shapePosition;
	}
	else
	{
		Camera::position = targetCameraPos;
	}



}

void Player::UpdateBody()
{

	if (EngineMain::MainInstance->SimulatingGameTicks) return;

	bodyAnimator.movementSpeed = length(MathHelper::XZ(velocity));

	vec3 playerForward = MathHelper::GetForwardVector(vec3(0, cameraRotation.y, 0));

	auto pose = bodyAnimator.GetResultPose();
	//pose.SetBoneTransform();

	if (InThirdPerson())
	{
		if (currentWeapon)
		{

			pose = currentWeapon->ApplyWeaponAnimation(pose);

		}
	}
	else
	{
		mat4 scale0 = scale(vec3(0));
		//pose.SetBoneTransform("neck_01", scale0);
		pose.SetBoneTransform("upperarm_r", scale0);
		pose.SetBoneTransform("upperarm_l", scale0);
	}


	bodyMesh->PasteAnimationPose(pose);
	bodyMesh->Position = Position - vec3(0, controller.height / 2.0f, 0) - playerForward * 0.3f;
	bodyMesh->Rotation.y = cameraRotation.y;

	if (InThirdPerson())
	{
		bodyMesh->Position = Position - vec3(0, controller.height / 2.0f, 0);
	}


	//std::unordered_map<std::string, mat4> poseT;
	//poseT["thigh_r"] = translate(Camera::position + Camera::Forward()) * scale(vec3(0.01f));
	//bodyMesh->ApplyWorldSpaceBoneTransforms(poseT);

	Camera::ApplyCameraShake(Time::DeltaTimeF);

	if (InThirdPerson())
	{
		UpdateThirdPersonCamera();
	}
	else
	{
		Camera::position = MathHelper::DecomposeMatrix(bodyMesh->GetBoneMatrixWorld("head")).Position + playerForward * 0.3f;

		float feetHeight = controller.GetSmoothPosition().y - controller.height / 2.0f;

		Camera::position.y = feetHeight + controller.GetCameraHeight();

		vec3 feetPos = controller.GetSmoothPosition();
		feetPos.y = feetHeight;

		float maxCameraHeight = 1.0f;

		auto hit = Physics::SphereTrace(feetPos + vec3(0, 0.5f, 0), feetPos + vec3(0, maxCameraHeight, 0), 0.2, BodyType::World);

		float distance = hit.fraction * (maxCameraHeight - 0.5f) + 0.5f;

		controller.cameraHeightCrouching = distance;

	}



	observationTarget->position = Position + vec3(0, 0.65f, 0);


	Physics::SetBodyPosition(hitbox, Camera::position - vec3(0, 0.5f, 0));

}

bool Player::InThirdPerson()
{

	if (currentWeapon)
	{
		return ThirdPersonView && !currentWeapon->ForceFirstPerson;
	}

	return ThirdPersonView;
}



void Player::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
	Entity::OnDamage(Damage, DamageCauser, Weapon);
}

void Player::OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon)
{
	Entity::OnPointDamage(Damage, Point, Direction, bone, DamageCauser, Weapon);

	vec3 right = Camera::Right();
	vec3 forward = Camera::Forward();

	float hitFromRight = dot(Direction, right);
	float hitFromFront = dot(Direction, forward);

	// Damage scaling (clamped)
	float damageScale = lerp(glm::clamp(Damage / 6.0f, 0.02f, 1.0f), 0.2f, 0.2f);

	vec3 rotationAmplitude;
	rotationAmplitude.x = -hitFromFront * 4.0f * damageScale; // pitch
	rotationAmplitude.y = 0.0f;                                // yaw (unused)
	rotationAmplitude.z = -hitFromRight * 6.0f * damageScale;  // roll

	CameraShake damageShake(
		0.05f,                    // interp in (snappy)
		0.6f,                    // duration
		vec3(0.0f),               // position amplitude (none)
		vec3(0.0f),               // position frequency
		rotationAmplitude,        // rotation amplitude (degrees)
		vec3(15.4f),              // rotation frequency (sharp)
		1.0f,                     // falloff
		CameraShake::SingleWave
	);

	Camera::AddCameraShake(damageShake);

	//GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point - vec3(0,0.5f,0), MathHelper::FindLookAtRotation(vec3(0), -Direction - vec3(0, 1, 0)), vec3(0.2f));

}

void Player::UpdateCurrentRestrictedArea()
{

	int currentAreaLevel = 0;

	auto results = Physics::PointTrace(Position, BodyType::Area1);

	for (auto result : results)
	{

		RestrictedArea* area = dynamic_cast<RestrictedArea*>(result.entity);

		if (area == nullptr) continue;

		if (area->RestrictionLevel > currentAreaLevel)
			currentAreaLevel = area->RestrictionLevel;

	}

	CurrentMaxRestrictionLevel = currentAreaLevel;

}

void Player::Serialize(json& target)
{

	Entity::Serialize(target);

	SERIALIZE_FIELD(target, cameraRotation);
	SERIALIZE_FIELD(target, velocity);
	SERIALIZE_FIELD(target, currentSlot);
	SERIALIZE_FIELD(target, weaponSlots);

	SERIALIZE_FIELD(target, NpcSimulationManager::worldSimulationState);

	SERIALIZE_FIELD(target, offhandWeapons);
	SERIALIZE_FIELD(target, offhandWeapon);
	SERIALIZE_FIELD(target, desiredOffhandWeapon);

	// Serialize inventory system
	target["weaponSystemMode"] = static_cast<int>(weaponSystemMode);
	SERIALIZE_FIELD(target, inventory);
	SERIALIZE_FIELD(target, currentInventoryUUID);
	SERIALIZE_FIELD(target, lastInventoryUUID);

	SERIALIZE_FIELD(target, currentMainWeaponUUID);
	SERIALIZE_FIELD(target, currentOffhandWeaponUUID);

}

void Player::Deserialize(json& source)
{

	Entity::Deserialize(source);

	DESERIALIZE_FIELD(source, cameraRotation);
	DESERIALIZE_FIELD(source, velocity);
	DESERIALIZE_FIELD(source, currentSlot);
	DESERIALIZE_FIELD(source, weaponSlots);

	DESERIALIZE_FIELD(source, NpcSimulationManager::worldSimulationState);

	DESERIALIZE_FIELD(source, offhandWeapons);
	DESERIALIZE_FIELD(source, offhandWeapon);
	DESERIALIZE_FIELD(source, desiredOffhandWeapon);

	DESERIALIZE_FIELD(source, currentMainWeaponUUID);
	DESERIALIZE_FIELD(source, currentOffhandWeaponUUID);

	// Deserialize inventory system
	if (source.contains("weaponSystemMode"))
	{
		weaponSystemMode = static_cast<WeaponSystemMode>(source["weaponSystemMode"].get<int>());
	}
	DESERIALIZE_FIELD(source, inventory);
	DESERIALIZE_FIELD(source, currentInventoryUUID);
	DESERIALIZE_FIELD(source, lastInventoryUUID);

	// Restore weapon based on mode
	if (weaponSystemMode == WeaponSystemMode::Slots)
	{
		SwitchToSlot(currentSlot, true);
	}
	else if (weaponSystemMode == WeaponSystemMode::Inventory)
	{

		for (auto& item : inventory)
		{
			item.mainWeaponData.inventoryUUID = item.uid;
			item.offhandWeaponData.inventoryUUID = item.uid;
		}

		currentInventoryUUID = "";

		if (currentMainWeaponUUID.empty() == false)
		{

			std::string mainWeaponUUIDToSwitch = currentMainWeaponUUID;
			DestroyWeapon();
			currentMainWeaponUUID = "";

			SwitchToInventoryItem(mainWeaponUUIDToSwitch, true);
		}

		if (currentOffhandWeaponUUID.empty() == false)
		{

			std::string offhandWeaponUUIDToSwitch = currentOffhandWeaponUUID;
			DestroyWeaponOffhand();
			currentOffhandWeaponUUID = "";

			SwitchToInventoryItem(offhandWeaponUUIDToSwitch, true);
		}

	}


	controller.SetVelocity(velocity);
	Teleport(Position);


}

void Player::Teleport(vec3 target)
{

	teleported = true;

	Position = target;
	oldPos = Position;

	controller.SetPosition(target);
	controller.heightSmoothOffset = 0;


}

void Player::MoveTo(vec3 target)
{

	auto hit = Physics::SphereTrace(Position, target, 0.2f, BodyType::World | BodyType::WorldSkybox);

	if (hit.hasHit)
	{

		target = hit.shapePosition;

	}

	Position = target;

	controller.SetPosition(target);
	controller.heightSmoothOffset = 0;

}

void Player::StartBike()
{
	if (on_bike) return;

	bikeMesh->PlayAnimation("draw", true, 0.7f);
	on_bike = true;
}

void Player::StopBike()
{
	if (on_bike == false) return;

	bikeMesh->PlayAnimation("hide", true, 0.7f);
	on_bike = false;
}

void Player::ToggleBike()
{
	if (on_bike)
	{
		StopBike();
	}
	else
	{
		StartBike();
	}
}

void Player::LoadAssets()
{
	bikeMesh->LoadFromFile("GameData/models/player/bike/bike.glb");
	bikeMesh->TexturesLocation = "GameData/models/player/bike/textures/";
	bikeMesh->PreloadAssets();
	bikeMesh->PlayAnimation("hide", true);

	bikeArmsMesh->LoadFromFile("GameData/arms.glb");
	bikeArmsMesh->PreloadAssets();

	bodyMesh->LoadFromFile("GameData/models/player/body/player_body.glb");
	//bodyMesh->LoadFromFile("GameData/models/npc/guard.glb/");
	bodyMesh->DepthPrePath = false;
	bodyMesh->Masked = true;
	//bodyMesh->TexturesLocation = "GameData/models/npc/guard.glb/";
	bodyMesh->PreloadAssets();

	bodyAnimator.LoadAssetsIfNeeded();

}
