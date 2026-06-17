#include "RemotePlayer.h"
#include "Player.hpp"
#include <Network/NetworkManager.h>
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
		if (referencePlayer)
		{
			targetPosition = referencePlayer->Position;
			targetRotation = referencePlayer->Rotation;
			playerHeight = referencePlayer->controller.isCrouched ? referencePlayer->controller.crouchHeight : referencePlayer->controller.height;

			// Grab the actual authoritative velocity to prepare it for NetSerialize.
			// (Adjust "Velocity" to match your referencePlayer's actual velocity property name).
			predictedVelocity = referencePlayer->controller.GetVelocity();
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
		// FIX: Use your dedicated correction speed constant instead of the server tick rate
		Position = MathHelper::Interp(Position, predictedPosition, Time::DeltaTimeF, kCorrectionInterpSpeed);
	}

	// Rotation can be updated directly, or smoothed if your game benefits from it.
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

	animator->movementSpeed = length(MathHelper::XZ(predictedVelocity));
	animator->Update();
	auto pose = animator->GetResultPose();
	mesh->PasteAnimationPose(pose);
}

void RemotePlayer::NetSerialize(NetPacket& packet)
{
	packet.WriteVector3(targetPosition);
	packet.WriteVector3(targetRotation);
	packet.WriteFloat(playerHeight);


	packet.WriteVector3(predictedVelocity);
}

void RemotePlayer::NetDeserialize(NetPacket& packet)
{
	const vec3 newTargetPosition = packet.ReadVector3();
	const vec3 newTargetRotation = packet.ReadVector3();
	playerHeight = packet.ReadFloat();

	// FIX: Read the velocity from the packet
	const vec3 incomingVelocity = packet.ReadVector3();

	lastNetPosition = targetPosition;
	targetPosition = newTargetPosition;
	targetRotation = newTargetRotation;

	// FIX: Assign it directly, completely bypassing local time jitter errors
	predictedVelocity = incomingVelocity;

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

	animator = new PlayerBodyAnimator(this);
	animator->LoadAssetsIfNeeded();
}

REGISTER_ENTITY(RemotePlayer, "remotePlayer")