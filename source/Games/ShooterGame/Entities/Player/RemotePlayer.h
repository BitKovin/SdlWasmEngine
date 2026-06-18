#pragma once

#include <Network/NetworkedEntity.h>
#include <SkeletalMesh.hpp>
#include <AiPerception/ObservationTarget.h>
#include "PlayerBodyAnimator.h"

class RemotePlayer : public NetworkedEntity
{
public:
	RemotePlayer();
	~RemotePlayer();

	class Player* referencePlayer = nullptr;
	SkeletalMesh* mesh = nullptr;

	SkeletalMesh* weaponR = nullptr;
	SkeletalMesh* weaponL = nullptr;

	SkeletalMesh* weaponAnimation = nullptr;

	PlayerBodyAnimator* animator = nullptr;

	std::shared_ptr<ObservationTarget> observationTarget;

	vec3 targetPosition{};
	vec3 targetRotation{};

	vec3 cameraRotation{};

	// Prediction
	vec3 lastNetPosition{};
	vec3 predictedVelocity{};
	float timeSinceNetUpdate = 0.0f;

	uint16_t weaponRIndex = INT16_MAX;
	uint16_t weaponLIndex = INT16_MAX;

	bool weaponRAkimbo = false;

	std::string weaponRModelPath = "";
	std::string weaponLModelPath = "";

	float playerHeight = 0;

	void Update();
	void AsyncUpdate();

	void NetSerialize(NetPacket& packet) override;
	void NetDeserialize(NetPacket& packet) override;
	void LoadAssets() override;

	AnimationPose ApplyWeaponAnimation(AnimationPose pose);

	void UpdateWeaponMeshes();

	void RecalculateWeaponPaths();

	uint16_t GetWeaponIndexFromRef(Entity* ent);

	std::string GetClassNameFromId(uint16_t id);


};