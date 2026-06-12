#pragma once

#include <Network/NetworkedEntity.h>
#include <SkeletalMesh.hpp>


class RemotePlayer : public NetworkedEntity
{
public:

	RemotePlayer();

	class Player* referencePlayer = nullptr;

	SkeletalMesh* mesh = nullptr;

	vec3 targetPosition{};
	vec3 targetRotation{};

	void Update();

	void NetSerialize(NetPacket& packet) override;

	void NetDeserialize(NetPacket& packet) override;

	void LoadAssets() override;

};

