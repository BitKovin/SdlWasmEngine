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

	PlayerBodyAnimator* animator = nullptr;

	std::shared_ptr<ObservationTarget> observationTarget;

	vec3 targetPosition{};
	vec3 targetRotation{};

	// Prediction
	vec3 lastNetPosition{};
	vec3 predictedVelocity{};
	float timeSinceNetUpdate = 0.0f;

	float playerHeight = 0;

	void Update();
	void AsyncUpdate();

	void NetSerialize(NetPacket& packet) override;
	void NetDeserialize(NetPacket& packet) override;
	void LoadAssets() override;
};