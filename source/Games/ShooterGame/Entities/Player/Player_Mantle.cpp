#include "Player.hpp"

#include <Helpers/CubicBezierEasing.hpp>

void Player::TryMantle()
{
	// ── Guard 1: state / cooldown ─────────────────────────────────────────────
	if (IsMantling())       return;
	if (mantleDelay.Wait()) return;   // cooldown between mantle attempts

	// ── Guard 2: must be airborne ─────────────────────────────────────────────
	if (OnGround()) return;

	// ── Guard 3: do not grab while plummeting ─────────────────────────────────
	// Allows mantling while ascending, at the apex, or during a slow descent.
	// A hard fall (-6 m/s threshold) almost certainly overshoots the ledge anyway.
	vec3 vel = controller.GetVelocity();
	if (vel.y < -60.0f) return;

	// ── Shared geometry values used across multiple guards ────────────────────
	const float halfHeight = controller.height / 2.0f;
	const float capsuleRadius = 0.38f;  // slightly inside the controller radius (0.4)
	const float playerFeetY = Position.y - halfHeight;

	const BodyType colMask = BodyType::GroupCollisionTest & ~BodyType::CharacterCapsule;
	const std::vector<Entity*> exclude = { this };

	// Horizontal forward direction — uses camera yaw only so the detection is
	// always level regardless of where the player is looking up or down.
	vec3 forward = glm::normalize(
		MathHelper::XZ(MathHelper::GetForwardVector(vec3(0, cameraRotation.y, 0)))
	);

	// ── Guard 4: forward wall detection ──────────────────────────────────────
	vec3 castOrigin = Position;
	vec3 castEnd = castOrigin + forward * MantleForwardReach;

	auto wallHit = Physics::SphereTrace(
		castOrigin, castEnd, 0.35f, colMask, {}, exclude);

	if (!wallHit.hasHit)
	{
		wallHit = Physics::SphereTrace(
			castOrigin + vec3(0, halfHeight - 0.2, 0), castEnd + vec3(0, halfHeight - 0.2, 0), 0.35f, colMask, {}, exclude);

		if (!wallHit.hasHit)
		{
			wallHit = Physics::SphereTrace(
				castOrigin - vec3(0, halfHeight - 0.2, 0), castEnd - vec3(0, halfHeight - 0.4, 0), 0.35f, colMask, {}, exclude);

			if (!wallHit.hasHit) return;

		}

	}

	// Surface must be roughly vertical — reject ramps and ceilings.
	if (wallHit.normal.y > 0.35f) return;

	// ── Guard 5: wall must face the player ────────────────────────────────────
	vec3 wallNormalXZ = MathHelper::XZ(wallHit.normal);
	float wallNormalLen = glm::length(wallNormalXZ);
	if (wallNormalLen < 0.01f) return;
	wallNormalXZ = glm::normalize(wallNormalXZ);

	if (glm::dot(wallNormalXZ, -forward) < 0.5f) return;

	// Reject if the player is actively moving away from the wall.
	vec3 horVel = MathHelper::XZ(vel);
	if (glm::length(horVel) > 0.5f)
	{
		if (glm::dot(glm::normalize(horVel), -wallNormalXZ) < -0.8f) return;
	}

	// ── Guard 6: ledge top detection ─────────────────────────────────────────
	vec3 probeXZ = MathHelper::XZ(wallHit.position)
		- wallNormalXZ * (capsuleRadius + 0.1f);

	float probeTopY = playerFeetY + MantleMaxLedgeHeight + 0.5f;
	float probeBotY = playerFeetY + MantleMinLedgeHeight - 0.2f;

	vec3 probeStart = vec3(probeXZ.x, probeTopY, probeXZ.z);
	vec3 probeEnd = vec3(probeXZ.x, probeBotY, probeXZ.z);

	auto ledgeHit = Physics::LineTrace(
		probeStart, probeEnd, colMask, {}, exclude);

	if (!ledgeHit.hasHit) return;

	// Ledge surface must be mostly flat — reject sloped overhangs.
	if (ledgeHit.normal.y < 0.7f) return;

	vec3 ledgeTop = ledgeHit.position;

	// ── Guard 7: height window ────────────────────────────────────────────────
	float ledgeRelH = ledgeTop.y - playerFeetY;
	if (ledgeRelH < MantleMinLedgeHeight) return;
	if (ledgeRelH > MantleMaxLedgeHeight) return;

	// ── Compute snap and target positions ────────────────────────────────────
	vec3 snapXZ = MathHelper::XZ(wallHit.position)
		+ wallNormalXZ * (capsuleRadius + 0.02f);

	float snapY = ledgeTop.y - 0.68f;

	if (snapY < Position.y - 0.2f) return;

	vec3 snapPosition = vec3(snapXZ.x, snapY, snapXZ.z);

	float overEdgePush = capsuleRadius * 0.5f;
	vec3  targetXZ = probeXZ - wallNormalXZ * overEdgePush;
	float targetY = ledgeTop.y + halfHeight + 0.02f;
	vec3  targetPosition = vec3(targetXZ.x, targetY, targetXZ.z);

	// ── Guard 8: capsule clearance ────────────────────────────────────────────
	{
		vec3 ceilCheckBot = vec3(targetXZ.x, ledgeTop.y + capsuleRadius, targetXZ.z);
		vec3 ceilCheckTop = vec3(targetXZ.x,
			ledgeTop.y + controller.height + MantleStandClearance,
			targetXZ.z);
		if (Physics::SphereTrace(ceilCheckBot, ceilCheckTop,
			capsuleRadius * 0.9f, colMask, {}, exclude).hasHit)
			return;
	}

	{
		vec3 snapBot = snapPosition - vec3(0, halfHeight - capsuleRadius, 0);
		vec3 snapTop = snapPosition + vec3(0, halfHeight - capsuleRadius, 0);
		if (Physics::SphereTrace(snapBot, snapTop,
			capsuleRadius * 0.9f, colMask, {}, exclude).hasHit)
			return;
	}

	{
		vec3 arcMid = vec3(
			glm::mix(snapPosition.x, targetPosition.x, 0.5f),
			ledgeTop.y + capsuleRadius + 0.05f,
			glm::mix(snapPosition.z, targetPosition.z, 0.5f)
		);
		vec3 arcMidTop = arcMid + vec3(0, halfHeight - capsuleRadius + 0.1f, 0);

		if (Physics::SphereTrace(arcMid, arcMidTop,
			capsuleRadius * 0.9f, colMask, {}, exclude).hasHit)
			return;
	}

	// ── All guards passed ─────────────────────────────────────────────────────
	mantleSnapPosition = controller.GetSmoothPosition();
	mantleTargetPosition = targetPosition;
	StartMantle();
}

// ---------------------------------------------------------------------------
// StartMantle
// ---------------------------------------------------------------------------
void Player::StartMantle()
{
	// Set state before anything else so IsMantling() reads true immediately.
	moveState = MoveState::Mantling;
	mantleProgress = 0.0f;
	mantleStartPosition = mantleSnapPosition;

	controller.SetPosition(mantleSnapPosition);
	controller.heightSmoothOffset = 0;
	Position = mantleSnapPosition;
	teleported = true;

	controller.SetVelocity(vec3(0));

	jumpDelay.AddDelay(MantleDuration + 0.15f);

	controller.UnCrouch();
	StopSlide();  // StopSlide is a no-op when not sliding; harmless here.
}

// ---------------------------------------------------------------------------
// UpdateMantle
// ---------------------------------------------------------------------------
void Player::UpdateMantle()
{
	// ── Jump cancel ───────────────────────────────────────────────────────────
	if (Input::GetAction("jump")->Pressed() && false)
	{
		vec3 fwd = glm::normalize(
			MathHelper::XZ(MathHelper::GetForwardVector(vec3(0, cameraRotation.y, 0))));

		controller.SetVelocity(vec3(fwd.x * 4.0f, 8.5f, fwd.z * 4.0f));
		jumpDelay.AddDelay(0.3f);
		FinishMantle(false);
		return;
	}

	// ── Advance progress ──────────────────────────────────────────────────────
	mantleProgress += Time::DeltaTimeF / MantleDuration;
	mantleProgress = glm::clamp(mantleProgress, 0.0f, 1.0f);

	float mantleEase = glm::smoothstep(0.0f, 1.0f, mantleProgress);
	mantleEase = glm::mix(glm::smoothstep(0.0f, 1.0f, mantleEase), mantleEase, 0.2f);

	CubicBezierEasing mantleBezier = CubicBezierEasing(.15, -0.2, .83, .89);

	float yPhase = mantleBezier(glm::clamp(mantleEase / 0.65f, 0.0f, 1.0f));
	float xzPhase = mantleBezier(glm::clamp((mantleEase - 0.35f) / 0.65f, 0.0f, 1.0f));

	vec3 pos;
	pos.y = glm::mix(mantleSnapPosition.y, mantleTargetPosition.y, yPhase);
	pos.x = glm::mix(mantleSnapPosition.x, mantleTargetPosition.x, xzPhase);
	pos.z = glm::mix(mantleSnapPosition.z, mantleTargetPosition.z, xzPhase);

	controller.SetPosition(pos);
	controller.SetVelocity(vec3(0));
	controller.heightSmoothOffset = 0;

	// ── Finish ────────────────────────────────────────────────────────────────
	if (mantleProgress >= 1.0f)
		FinishMantle(true);
}

// ---------------------------------------------------------------------------
// FinishMantle
// ---------------------------------------------------------------------------
void Player::FinishMantle(bool isNaturalFinish)
{
	// Clear state first so IsMantle() reads false from this point on.
	moveState = MoveState::Default;
	mantleProgress = 0.0f;

	if (isNaturalFinish)
	{
		controller.SetPosition(mantleTargetPosition);
		Position = mantleTargetPosition;
		oldPos = Position;

		vec3 fwd = glm::normalize(
			MathHelper::XZ(MathHelper::GetForwardVector(vec3(0, cameraRotation.y, 0))));
		controller.SetVelocity(fwd * 2.0f);
	}

	mantleDelay.AddDelay(MantleCooldown);

	freeWalljumps = 1;
}