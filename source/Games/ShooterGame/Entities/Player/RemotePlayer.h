#pragma once

#include <Network/NetworkedEntity.h>
#include <SkeletalMesh.hpp>
#include <AiPerception/ObservationTarget.h>

class RemotePlayer : public NetworkedEntity
{
public:
	RemotePlayer();

	class Player* referencePlayer = nullptr;
	SkeletalMesh* mesh = nullptr;

	std::shared_ptr<ObservationTarget> observationTarget;

	vec3 targetPosition{};
	vec3 targetRotation{};

	// Prediction
	vec3 lastNetPosition{};
	vec3 predictedVelocity{};
	float timeSinceNetUpdate = 0.0f;

	void Update();

	void NetSerialize(NetPacket& packet) override;
	void NetDeserialize(NetPacket& packet) override;
	void LoadAssets() override;
};