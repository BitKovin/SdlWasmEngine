#include "RemotePlayer.h"

#include "Player.hpp"

RemotePlayer::RemotePlayer()
{

	ClassName = "remotePlayer";

}

void RemotePlayer::Update()
{

	Visible = !isOwned;

	if (isOwned)
	{
		targetPosition = referencePlayer->Position;
		targetRotation = referencePlayer->Rotation;
	}

	Position = MathHelper::Interp(Position, targetPosition, Time::DeltaTimeF, 19);
	Rotation = targetRotation;// MathHelper::Interp(Rotation, targetRotation, Time::DeltaTimeF, 15);

	if (distance(Position, targetPosition) > 2.5f)
	{
		Position = targetPosition;
	}

	mesh->Position = Position - vec3(0,0.9f,0);
	mesh->Rotation = Rotation;

}

void RemotePlayer::NetSerialize(NetPacket& packet)
{


	packet.WriteVector3(targetPosition);
	packet.WriteVector3(targetRotation);
}

void RemotePlayer::NetDeserialize(NetPacket& packet)
{


	targetPosition = packet.ReadVector3();
	targetRotation = packet.ReadVector3();

}

void RemotePlayer::LoadAssets()
{

	mesh = new SkeletalMesh(this);
	Drawables.push_back(mesh);

	mesh->LoadFromFile("GameData/models/player/body/player_body.glb");
	mesh->GravityAlignedRotation = true;
	mesh->DepthPrePath = false;
	mesh->Masked = true;
	mesh->PreloadAssets();

}

REGISTER_ENTITY(RemotePlayer,"remotePlayer")