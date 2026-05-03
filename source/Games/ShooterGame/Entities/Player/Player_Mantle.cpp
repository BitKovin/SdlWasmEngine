#include "Player.hpp"

#include <Helpers/CubicBezierEasing.hpp>

void Player::TryMantle()
{
	// ── Guard 1: state / cooldown ─────────────────────────────────────────────
	if (isMantling)       return;
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
	// Cast origin is at chest height (30 % above capsule center) so the trace
	// isnates from a point that could physically press against a ledge face.
	// A sphere trace (radius 0.15) is more forgiving than a line for slightly
	// uneven wall geometry.
	vec3 castOrigin = Position;// +vec3(0, halfHeight - 0.3f, 0);
	vec3 castEnd = castOrigin + forward * MantleForwardReach;

	auto wallHit = Physics::SphereTrace(
		castOrigin, castEnd, 0.35f, colMask, {}, exclude);

	if (!wallHit.hasHit)
	{
		wallHit = Physics::SphereTrace(
			castOrigin + vec3(0,halfHeight - 0.2,0) , castEnd + vec3(0, halfHeight - 0.2, 0), 0.35f, colMask, {}, exclude);

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
	// Project the wall normal onto the horizontal plane and compare with the
	// player's inverse forward direction.  dot < 0.5 means the wall is more
	// than 60° off-axis — the player would clip through rather than grab it.
	vec3 wallNormalXZ = MathHelper::XZ(wallHit.normal);
	float wallNormalLen = glm::length(wallNormalXZ);
	if (wallNormalLen < 0.01f) return;   // degenerate normal (ceiling-adjacent face)
	wallNormalXZ = glm::normalize(wallNormalXZ);

	if (glm::dot(wallNormalXZ, -forward) < 0.5f) return;

	// Reject if the player is actively moving away from the wall.
	// Allows strafing toward the wall (dot near 0) but not retreating (dot < -1).
	vec3 horVel = MathHelper::XZ(vel);
	if (glm::length(horVel) > 0.5f)
	{
		if (glm::dot(glm::normalize(horVel), -wallNormalXZ) < -0.8f) return;
	}

	// ── Guard 6: ledge top detection ─────────────────────────────────────────
	// We know where the wall face is in XZ (wallHit.position).  The ledge top
	// is the horizontal surface sitting on top of that wall.  To find it, probe
	// downward from above the max mantle height at a point slightly behind the
	// wall face (on the far side the player will land on).
	//
	// "Behind the wall face" = moving in the -wallNormal direction past the face
	// by (capsuleRadius + 0.1) so the probe is fully inside the solid and will
	// find the top surface on its way down.
	vec3 probeXZ = MathHelper::XZ(wallHit.position)
		- wallNormalXZ * (capsuleRadius + 0.1f);

	float probeTopY = playerFeetY + MantleMaxLedgeHeight + 0.5f;  // start above window
	float probeBotY = playerFeetY + MantleMinLedgeHeight - 0.2f;  // end below window

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
	//
	// SNAP POSITION — the capsule-center the player is teleported to at the
	// instant the mantle begins (the "hanging from the ledge" pose).
	//
	//   XZ: place the capsule face flush against the wall on the player's side.
	//       wallHit.position is the exact face contact; stepping back by
	//       capsuleRadius gives the capsule center without overlap.
	//
	//   Y:  the camera (at Position + 0.7) should sit at ledge height so the
	//       player appears to be grabbing the top.  Solving:
	//         snap.y + 0.68 ≈ ledgeTop.y  →  snap.y = ledgeTop.y - 0.68
	//       (0.68 rather than 0.7 to leave the eyes just at ledge level)
	//
	vec3 snapXZ = MathHelper::XZ(wallHit.position)
		+ wallNormalXZ * (capsuleRadius + 0.02f);

	float snapY = ledgeTop.y - 0.68f;

	// Do not pull the player downward — if the snap Y would be below their
	// current position something is geometrically unusual; skip this ledge.
	if (snapY < Position.y - 0.2f) return;

	vec3 snapPosition = vec3(snapXZ.x, snapY, snapXZ.z);

	//
	// TARGET POSITION — capsule-center when the player is standing on the ledge.
	//
	//   XZ: the probe point is already on the far side of the ledge edge, so use
	//       it directly.  A small push deeper onto the platform prevents landing
	//       right on the edge.
	//
	//   Y:  center = surface + halfHeight + tiny epsilon (not embedded in surface)
	//
	float overEdgePush = capsuleRadius * 0.5f;
	vec3  targetXZ = probeXZ - wallNormalXZ * overEdgePush;
	float targetY = ledgeTop.y + halfHeight + 0.02f;
	vec3  targetPosition = vec3(targetXZ.x, targetY, targetXZ.z);

	// ── Guard 8: capsule clearance ────────────────────────────────────────────
	// Three checks, cheapest first:
	//
	//   8a. Space above the ledge for the player to stand (ceiling clearance).
	//       A single sphere trace sweeping from the ledge surface up through the
	//       full player height catches low-ceiling mantles.
	//
	{
		vec3 ceilCheckBot = vec3(targetXZ.x, ledgeTop.y + capsuleRadius, targetXZ.z);
		vec3 ceilCheckTop = vec3(targetXZ.x,
			ledgeTop.y + controller.height + MantleStandClearance,
			targetXZ.z);
		if (Physics::SphereTrace(ceilCheckBot, ceilCheckTop,
			capsuleRadius * 0.9f, colMask, {}, exclude).hasHit)
			return;
	}

	//   8b. Space at the snap (hang) position — ensures the player's capsule
	//       fits while hanging.  Sweeps from capsule bottom to top.
	//
	{
		vec3 snapBot = snapPosition - vec3(0, halfHeight - capsuleRadius, 0);
		vec3 snapTop = snapPosition + vec3(0, halfHeight - capsuleRadius, 0);
		if (Physics::SphereTrace(snapBot, snapTop,
			capsuleRadius * 0.9f, colMask, {}, exclude).hasHit)
			return;
	}

	//   8c. Arc midpoint — the highest point of the mantle arc where the player
	//       is at ledge height transitioning from pulling up to vaulting over.
	//       If there is a lip, beam, or low ceiling here the mantle must be
	//       blocked; going through would look like clipping and could trap the
	//       player.
	//
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
//
// Called by TryMantle() after all checks pass.
// Teleports the player to the snap (hang) position, kills all physics
// velocity, and sets up the animation state.
// ---------------------------------------------------------------------------
void Player::StartMantle()
{
	isMantling = true;
	mantleProgress = 0.0f;
	mantleStartPosition = mantleSnapPosition;

	// Teleport capsule to the hang position.
	// Mirror what Teleport() does so the position-change guard in Update() and
	// the camera height smoother do not fight the new position.
	controller.SetPosition(mantleSnapPosition);
	controller.heightSmoothOffset = 0;
	Position = mantleSnapPosition;
	teleported = true;   // suppress the inter-frame movement safety check in Update()

	// Kill all velocity.  The CharacterController will still apply gravity
	// internally, but UpdateMantle() re-applies SetPosition + SetVelocity(0)
	// every frame so the tiny per-frame drift is imperceptible.
	controller.SetVelocity(vec3(0));

	// Suppress OnGround() for the full mantle duration + a small margin.
	// This prevents coyoteTime from firing mid-air and stops Jump() from
	// being accepted as a ground jump while the animation plays.
	jumpDelay.AddDelay(MantleDuration + 0.15f);

	// The player is crouch-unblocked for the mantle regardless of previous state.
	controller.UnCrouch();
	StopSlide();
}

// ---------------------------------------------------------------------------
// UpdateMantle
//
// Drives the mantle animation every frame while isMantling is true.
// Called at the top of UpdateWalkMovement(), which returns immediately
// after, preventing all other movement code from running.
//
// Animation curve (two-phase with overlap):
//   Phase Y   [t=0.00 … 0.65]  → pull-up    (rises to ledge height)
//   Phase XZ  [t=0.35 … 1.00]  → vault-over (moves onto the platform)
//
// The 30 % overlap (t 0.35–0.65) produces a natural diagonal arc at the peak
// rather than a hard two-axis sequence.  Both phases use smoothstep so they
// ease in and out.
//
// Jump cancel: pressing jump during a mantle launches the player upward from
// their current position, useful for reaching ceilings or chaining moves.
// ---------------------------------------------------------------------------
void Player::UpdateMantle()
{
	// ── Jump cancel ───────────────────────────────────────────────────────────
	if (Input::GetAction("jump")->Pressed() && false)
	{
		vec3 fwd = glm::normalize(
			MathHelper::XZ(MathHelper::GetForwardVector(vec3(0, cameraRotation.y, 0))));

		// Launch velocity is set before FinishMantle so it isn't overwritten.
		controller.SetVelocity(vec3(fwd.x * 4.0f, 8.5f, fwd.z * 4.0f));
		jumpDelay.AddDelay(0.3f);
		FinishMantle(false /*cancel, not natural finish*/);
		return;
	}

	// ── Advance progress ──────────────────────────────────────────────────────
	mantleProgress += Time::DeltaTimeF / MantleDuration;
	mantleProgress = glm::clamp(mantleProgress, 0.0f, 1.0f);

	float mantleEase = glm::smoothstep(0.0f, 1.0f, mantleProgress);
	mantleEase = glm::mix(glm::smoothstep(0.0f, 1.0f, mantleEase), mantleEase, 0.2f);   // double-smooth for extra ease

	CubicBezierEasing mantleBezier = CubicBezierEasing(.15, -0.2, .83, .89);

	// Y phase: runs from t=0 to t=0.65 (pull-up)
	float yPhase = mantleBezier(glm::clamp(mantleEase / 0.65f, 0.0f, 1.0f));
	// XZ phase: runs from t=0.35 to t=1.0 (vault-over)
	float xzPhase = mantleBezier(glm::clamp((mantleEase - 0.35f) / 0.65f, 0.0f, 1.0f));


	vec3 pos;
	pos.y = glm::mix(mantleSnapPosition.y, mantleTargetPosition.y, yPhase);
	pos.x = glm::mix(mantleSnapPosition.x, mantleTargetPosition.x, xzPhase);
	pos.z = glm::mix(mantleSnapPosition.z, mantleTargetPosition.z, xzPhase);

	controller.SetPosition(pos);
	controller.SetVelocity(vec3(0));   // fight per-frame gravity integration
	controller.heightSmoothOffset = 0;


	// ── Finish ────────────────────────────────────────────────────────────────
	if (mantleProgress >= 1.0f)
		FinishMantle(true /*natural finish*/);
}

// ---------------------------------------------------------------------------
// FinishMantle
//
// Cleans up state after the animation completes (or is jump-cancelled).
// isNaturalFinish=true  → player completed the full arc, snap to target.
// isNaturalFinish=false → jump-cancel mid-arc, leave position where it is.
// Sets a small forward velocity so the player glides onto the platform
// naturally rather than snapping dead-still.
// ---------------------------------------------------------------------------
void Player::FinishMantle(bool isNaturalFinish)
{
	isMantling = false;
	mantleProgress = 0.0f;

	if (isNaturalFinish)
	{
		// Snap to the exact target position to counteract any floating-point
		// drift that accumulated over the animation frames.
		controller.SetPosition(mantleTargetPosition);
		Position = mantleTargetPosition;
		oldPos = Position;

		// Small forward push so the player glides onto the platform rather
		// than stopping dead the instant they land.
		vec3 fwd = glm::normalize(
			MathHelper::XZ(MathHelper::GetForwardVector(vec3(0, cameraRotation.y, 0))));
		controller.SetVelocity(fwd * 2.0f);
	}
	// For a cancel, velocity has already been set by the caller (jump-cancel
	// sets a full launch velocity before calling FinishMantle).

	// Start the cooldown timer.
	mantleDelay.AddDelay(MantleCooldown);

	// Restore one free wall-jump — reward the player for a successful mantle.
	freeWalljumps = 1;
}