#include "RemotePlayer.h"
#include "Player.hpp"
#include <Network/NetworkManager.h>
#include <AiPerception/AiPerceptionSystem.h>
#include "Weapons/WeaponFirearm.h"

namespace
{
	// How far the rendered position is allowed to drift before we snap.
	constexpr float kSnapDistance = 2.5f;

	// Correction smoothing toward the extrapolated position.
	constexpr float kCorrectionInterpSpeed = 18.0f;

	// Prevent runaway prediction if packets stall.
	constexpr float kMaxPredictionTime = 0.25f;
}

RemotePlayer::RemotePlayer()
{
	ClassName = "remotePlayer";

	Tags = { "player" };

	DestroyOnOwnerDisconnect = true;

	weaponR = new SkeletalMesh(this);
	Drawables.push_back(weaponR);
	weaponL = new SkeletalMesh(this);
	Drawables.push_back(weaponL);

}

RemotePlayer::~RemotePlayer()
{
	if (animator)
		delete animator;
}

void RemotePlayer::Update()
{
	Visible = !isOwned;

	if (isOwned)
	{
		if (Visible)
			RecalculateWeaponPaths();

		if (referencePlayer)
		{
			targetPosition = referencePlayer->Position;
			targetRotation = referencePlayer->Rotation;
			playerHeight = referencePlayer->controller.isCrouched ? referencePlayer->controller.crouchHeight : referencePlayer->controller.height;

			predictedVelocity = referencePlayer->controller.GetVelocity();

			weaponRIndex = GetWeaponIndexFromRef(referencePlayer->currentWeapon);
			weaponLIndex = GetWeaponIndexFromRef(referencePlayer->currentOffhandWeapon);

			weaponRAkimbo = false;

			if (referencePlayer->currentWeapon)
			{
				WeaponFirearm* weapFirearm = dynamic_cast<WeaponFirearm*>(referencePlayer->currentWeapon);

				if (weapFirearm)
					weaponRAkimbo = weapFirearm->akimbo; // FIX: Changed '==' to '='
			}

			cameraRotation = referencePlayer->cameraRotation;
		}

		// Local owner should not accumulate prediction state.
		timeSinceNetUpdate = 0.0f;

		Position = targetPosition;
		Rotation = targetRotation;

		if (observationTarget)
		{
			AiPerceptionSystem::RemoveTarget(observationTarget);
			observationTarget = nullptr;
		}
	}
	else
	{
		timeSinceNetUpdate += Time::DeltaTimeF;

		if (observationTarget == nullptr)
			observationTarget = AiPerceptionSystem::CreateTarget(Position, Id, { "player" });
	}

	// Extrapolate from the last received snapshot.
	const float predictionTime = (timeSinceNetUpdate > kMaxPredictionTime)
		? kMaxPredictionTime
		: timeSinceNetUpdate;

	vec3 predictedPosition = targetPosition + (predictedVelocity * predictionTime);

	// Interpolate only as a correction step, not as the primary motion path.
	const float errorDistance = distance(Position, predictedPosition);

	if (errorDistance > kSnapDistance)
	{
		Position = predictedPosition;
	}
	else
	{
		Position = MathHelper::Interp(Position, predictedPosition, Time::DeltaTimeF, kCorrectionInterpSpeed);
	}

	Rotation = targetRotation;

	if (mesh)
	{
		mesh->Position = Position - vec3(0, playerHeight / 2.0f, 0);
		mesh->Rotation = Rotation;
	}

	if (observationTarget)
	{
		observationTarget->position = Position + vec3(0, 0.7f, 0);
	}
}

void RemotePlayer::AsyncUpdate()
{
	if (animator == nullptr) return;

	UpdateWeaponMeshes();

	animator->movementSpeed = length(MathHelper::XZ(predictedVelocity));
	animator->Update();
	auto pose = animator->GetResultPose();

	pose = ApplyWeaponAnimation(pose);

	mesh->PasteAnimationPose(pose);

	if (weaponR->Visible)
		weaponR->PasteAnimationPose(pose);

	if(weaponL->Visible)
		weaponL->PasteAnimationPose(pose);

	weaponR->Position = weaponL->Position = mesh->Position;
	weaponR->Rotation = weaponL->Rotation = mesh->Rotation;
}

void RemotePlayer::NetSerialize(NetPacket& packet)
{
	packet.WriteVector3(targetPosition);
	packet.WriteVector3(targetRotation);
	packet.WriteFloat(playerHeight);
	packet.WriteVector3(predictedVelocity);
	packet.WriteVector3(cameraRotation);

	packet.WriteUInt16(weaponRIndex);
	packet.WriteUInt16(weaponLIndex);

	packet.WriteBool(weaponRAkimbo);
}

void RemotePlayer::NetDeserialize(NetPacket& packet)
{
	const vec3 newTargetPosition = packet.ReadVector3();
	const vec3 newTargetRotation = packet.ReadVector3();
	playerHeight = packet.ReadFloat();

	const vec3 incomingVelocity = packet.ReadVector3();

	lastNetPosition = targetPosition;
	targetPosition = newTargetPosition;
	targetRotation = newTargetRotation;

	predictedVelocity = incomingVelocity;

	timeSinceNetUpdate = 0.0f;

	cameraRotation = packet.ReadVector3();

	uint16_t newWeaponR = packet.ReadUInt16();
	uint16_t newWeaponL = packet.ReadUInt16();

	// NEW: Deserialize the akimbo state
	bool newAkimbo = packet.ReadBool();

	if (newWeaponR != weaponRIndex || newWeaponL != weaponLIndex)
	{
		weaponRIndex = newWeaponR;
		weaponLIndex = newWeaponL;

		RecalculateWeaponPaths();
	}


	weaponRAkimbo = newAkimbo;
}

void RemotePlayer::LoadAssets()
{
	mesh = new SkeletalMesh(this);
	Drawables.push_back(mesh);


	weaponR->MeshHideList = { "w_l" };
	weaponL->MeshHideList = { "w_r" };
	weaponL->TwoSided = true;

	weaponAnimation = new SkeletalMesh(this);
	weaponAnimation->LoadFromFile("GameData/animations/player/tp_weapons.glb");
	Drawables.push_back(weaponAnimation);//just to it gets cleaned with entity. doesn't gets drawn anyway

	weaponAnimation->PlayAnimation("weapon_rl",true,0);

	mesh->LoadFromFile("GameData/models/player/body/player_body.glb");
	mesh->GravityAlignedRotation = true;
	mesh->DepthPrePath = false;
	mesh->Masked = true;
	mesh->PreloadAssets();

	animator = new PlayerBodyAnimator(this);
	animator->LoadAssetsIfNeeded();
}

AnimationPose RemotePlayer::ApplyWeaponAnimation(AnimationPose pose) {
	// FIX: Check against UINT16_MAX so weapon index 0 (first in registry) isn't ignored
	if (weaponRIndex == UINT16_MAX && weaponLIndex == UINT16_MAX)
		return pose;

	AnimationPose outPose = pose;

	if (weaponRIndex != UINT16_MAX || weaponLIndex != UINT16_MAX)
	{
		outPose = AnimationPose::LayeredLerp("spine_03", weaponAnimation->GetRootNode(), pose, weaponAnimation->GetAnimationPose(), 1, 1);
	}

	outPose.boneTransforms["spine_02"] = outPose.boneTransforms["spine_02"]
		* MathHelper::GetRotationMatrix(
			vec3(0, 0, cameraRotation.x * -0.75f));

	return outPose;
}

void RemotePlayer::UpdateWeaponMeshes() {
	if (weaponR->filePath != weaponRModelPath)
	{
		weaponR->LoadFromFile(weaponRModelPath);
		weaponR->TexturesLocation = weaponRModelPath + "/";
		// FIX: Removed the static hide list assignment from here so it doesn't conflict
	}

	if (weaponL->filePath != weaponLModelPath)
	{
		weaponL->LoadFromFile(weaponLModelPath);
		weaponL->TexturesLocation = weaponLModelPath + "/";
		weaponL->MeshHideList = { "w_r" };
	}

	// NEW: Dynamically handle weaponR's hide list every frame based on Akimbo state
	if (weaponRAkimbo)
	{
		// Clear the hide list to show both parts
		if (!weaponR->MeshHideList.empty()) {
			weaponR->MeshHideList.clear();
		}
	}
	else
	{
		// Enforce hiding the left part of the weapon if not akimbo
		if (weaponR->MeshHideList.empty() || weaponR->MeshHideList.count("w_l") == 0) {
			weaponR->MeshHideList = { "w_l" };
		}
	}

	if (weaponR->filePath.empty())
		weaponR->Visible = false;

	if (weaponL->filePath.empty())
		weaponL->Visible = false;
}

void RemotePlayer::RecalculateWeaponPaths() {
	// FIX: Explicitly check against UINT16_MAX instead of casting IDs to booleans
	bool hasWeaponR = (weaponRIndex != UINT16_MAX);
	bool hasWeaponL = (weaponLIndex != UINT16_MAX);

	weaponR->Visible = hasWeaponR;
	weaponL->Visible = hasWeaponL;

	if (hasWeaponR)
	{
		std::string weapClassname = GetClassNameFromId(weaponRIndex);
		if (!weapClassname.empty())
		{
			auto weapEnt = LevelObjectFactory::instance().create(weapClassname);
			Weapon* weapPtr = dynamic_cast<Weapon*>(weapEnt);
			assert(weapPtr);

			WeaponFirearm* firearmPtr = dynamic_cast<WeaponFirearm*>(weapEnt);
			weaponRModelPath = firearmPtr ? firearmPtr->params.modelPathTp : weapPtr->thirdPersonModelPath;

			delete weapEnt;
		}
	}
	else
	{
		// FIX: Explicitly clear the path when unequipped
		weaponRModelPath = "";
		weaponR->filePath = "";
	}

	if (hasWeaponL)
	{
		std::string weapClassname = GetClassNameFromId(weaponLIndex);
		if (!weapClassname.empty())
		{
			auto weapEnt = LevelObjectFactory::instance().create(weapClassname);
			Weapon* weapPtr = dynamic_cast<Weapon*>(weapEnt);
			assert(weapPtr);

			WeaponFirearm* firearmPtr = dynamic_cast<WeaponFirearm*>(weapEnt);
			weaponLModelPath = firearmPtr ? firearmPtr->params.modelPathTp : weapPtr->thirdPersonModelPath;

			delete weapEnt;
		}
	}
	else
	{
		// FIX: Explicitly clear the path when unequipped
		weaponLModelPath = "";
		weaponL->filePath = "";
	}
}

map<std::string, uint16_t> weaponClassNameToIndexMap;
std::map<uint16_t, std::string> weaponIndexToClassNameMap;

uint16_t RemotePlayer::GetWeaponIndexFromRef(Entity* ent)
{
	if (ent == nullptr)
		return UINT16_MAX; // or 0, depending on your invalid value

	// Check cache first
	auto cached = weaponClassNameToIndexMap.find(ent->ClassName);
	if (cached != weaponClassNameToIndexMap.end())
		return cached->second;

	const auto& registry = LevelObjectFactory::instance().GetRegistry();

	auto it = registry.find(ent->ClassName);
	if (it == registry.end())
		return UINT16_MAX; // class not found

	uint16_t index = static_cast<uint16_t>(
		std::distance(registry.begin(), it));

	weaponClassNameToIndexMap.emplace(ent->ClassName, index);
	weaponIndexToClassNameMap.emplace(index, ent->ClassName);

	return index;
}

std::string RemotePlayer::GetClassNameFromId(uint16_t id)
{

	if (id == UINT16_MAX)
		return "";

	auto cached = weaponIndexToClassNameMap.find(id);
	if (cached != weaponIndexToClassNameMap.end())
		return cached->second;

	const auto& registry = LevelObjectFactory::instance().GetRegistry();

	if (id >= registry.size())
		return {};

	auto it = registry.begin();
	std::advance(it, id);

	const std::string& className = it->first;

	weaponIndexToClassNameMap.emplace(id, className);
	weaponClassNameToIndexMap.emplace(className, id);

	return className;
}

REGISTER_ENTITY(RemotePlayer, "remotePlayer")