#include "RemotePlayer.h"
#include "Player.hpp"

#include <NetworkManager.h>
#include <AiPerception/AiPerceptionSystem.h>

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

}

void RemotePlayer::Update()
{
	Visible = !isOwned;

	if (isOwned)
	{
		if (referencePlayer)
		{
			targetPosition = referencePlayer->Position;
			targetRotation = referencePlayer->Rotation;
		}

		// Local owner should not accumulate prediction state.
		timeSinceNetUpdate = 0.0f;
		predictedVelocity = vec3(0.0f);

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

		if(observationTarget == nullptr)
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
		Position = MathHelper::Interp(Position, predictedPosition, Time::DeltaTimeF, NetworkManager::GetTickRate());
	}

	// Rotation can be updated directly, or smoothed if your game benefits from it.
	Rotation = targetRotation;

	if (mesh)
	{
		mesh->Position = Position - vec3(0, 0.9f, 0);
		mesh->Rotation = Rotation;
	}

	if (observationTarget)
	{
		DebugDraw::Point(observationTarget->position);

		observationTarget->position = Position + vec3(0, 0.7f, 0);
	}


}

void RemotePlayer::NetSerialize(NetPacket& packet)
{
	packet.WriteVector3(targetPosition);
	packet.WriteVector3(targetRotation);
}

void RemotePlayer::NetDeserialize(NetPacket& packet)
{
	const vec3 newTargetPosition = packet.ReadVector3();
	const vec3 newTargetRotation = packet.ReadVector3();

	// Estimate velocity from the time between snapshots.
	if (timeSinceNetUpdate > 0.0001f)
	{
		const vec3 snapshotDelta = newTargetPosition - targetPosition;
		predictedVelocity = snapshotDelta / timeSinceNetUpdate;
	}
	else
	{
		predictedVelocity = vec3(0.0f);
	}

	lastNetPosition = targetPosition;
	targetPosition = newTargetPosition;
	targetRotation = newTargetRotation;
	timeSinceNetUpdate = 0.0f;
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

REGISTER_ENTITY(RemotePlayer, "remotePlayer")