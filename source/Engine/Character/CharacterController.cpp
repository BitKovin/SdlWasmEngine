#include "CharacterController.h"

#include <cassert>

// ---------------------------------------------------------------------------
// Static registry
// ---------------------------------------------------------------------------

std::vector<CharacterController*> CharacterController::s_allControllers;

// ---------------------------------------------------------------------------

CharacterController::CharacterController()
{
	currentCameraHeight = cameraHeightStanding;
	targetCameraHeight = cameraHeightStanding;
}

CharacterController::~CharacterController()
{
	s_allControllers.erase(
		std::remove(s_allControllers.begin(), s_allControllers.end(), this),
		s_allControllers.end());

	if (body)
		Physics::DestroyBody(body);
	if (sensorBody)
		Physics::DestroyBody(sensorBody);
}


void CharacterController::Init(Entity* owner, vec3 position, float radius, float height, float mass)
{
	this->owner = owner;
	this->standingHeight = height;

	Destroy();

	body = Physics::CreateCharacterBody(owner, position, radius, height - stepHeight, mass);
	body->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(body)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(body)->sensorCollisionMode = SensorCollisionMode::NotCollideWithSensors;

	// Sensor-only body: follows the character but only interacts with sensor volumes
	sensorBody = Physics::CreateCharacterCylinderBody(owner, position, radius, height, mass);
	sensorBody->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(sensorBody)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(sensorBody)->sensorCollisionMode = SensorCollisionMode::CollideOnlyWithSensors;
	Physics::SetGravityFactor(sensorBody, 0);

	this->height = height;
	this->radius = radius;

	Physics::SetGravityFactor(body, 0);

	// Register in the global controller list (guard against double-add on re-init)
	if (std::find(s_allControllers.begin(), s_allControllers.end(), this) == s_allControllers.end())
		s_allControllers.push_back(this);
}

void CharacterController::Destroy()
{
	// Clean up ignore pairs before the bodies are destroyed — BodyIDs become
	// invalid after DestroyBody so this must happen first.
	if (body)
	{
		for (const BodyID& id : activeIgnorePairs)
			Physics::RemoveIgnorePair(body->GetID(), id);
	}
	activeIgnorePairs.clear();

	if (body)
	{
		Physics::DestroyBody(body);
		body = nullptr;
	}
	if (sensorBody)
	{

		Physics::DestroyBody(sensorBody);
		sensorBody = nullptr;
	}
}

void CharacterController::Update(float deltaTime)
{
	if (body == nullptr) return;

	UpdateSmoothPosition(deltaTime);

	Entity* standingOnEntityPtr = standingOnEntity.Resolve(false);

	Body* lastStandingOnBodyPtr = Physics::GetBodyFromId(lastStandingOnBody);

	vec3 currentVelocity = GetVelocity();
	bool isIdle = glm::length(MathHelper::XZ(currentVelocity)) < 0.001f && currentVelocity.y <= 0;
	bool skipTracesAndMovement = (isIdle && onGround && standingOnEntityPtr != nullptr && standingOnEntityPtr->Static == true) && allowSleep;

	if (skipTracesAndMovement)
	{
		SetVelocity(vec3(0.0f));
	}
	else
	{
		// -------------------------------
		// 1) If previously attached, preemptively apply platform delta
		// -------------------------------
		if (lastStandingOnBodyPtr)
		{
			vec3 basePos = FromPhysics(lastStandingOnBodyPtr->GetPosition());
			glm::quat baseRot = FromPhysics(lastStandingOnBodyPtr->GetRotation());

			vec3 attachWorldPos = baseRot * baseLocalAttachPoint + basePos;
			vec3 platformDelta = attachWorldPos - prevAttachWorldPos;

			// clamp huge deltas to prevent teleport jumps
			const float maxDelta = 10.0f;
			if (glm::length(platformDelta) > maxDelta)
			{
				prevAttachWorldPos = attachWorldPos;
			}
			else
			{
				if (glm::length(platformDelta) > 1e-6f)
				{
					vec3 currentPos = FromPhysics(body->GetPosition());
					vec3 targetPos = currentPos + platformDelta;

					// sweep character to follow platform while resolving collisions
					Physics::SweepBody(body, targetPos, { owner });
				}

				prevAttachWorldPos = attachWorldPos;
			}

			prevBaseRotation = baseRot;
			prevBasePosition = basePos;
		}

		// -------------------------------
		// 2) Ground check after potential platform move
		// -------------------------------
		float verticalPosition;
		bool standsOnGround;
		vec3 walkableNormal = vec3();
		vec3 notWalkableNormal = vec3();
		UpdateGroundCheck(standsOnGround, verticalPosition, onGround, walkableNormal, notWalkableNormal);

		currentGroundNormal = walkableNormal;

		//DebugDraw::Line(GetPosition() - vec3(0, 1, 0) * height / 2.0f, GetPosition() - vec3(0, 1, 0) * height / 2.0f + walkableNormal);

		if (onGround == false)
		{
			standingOnBody = BodyID();
			standingOnEntity = EntityHandle();
		}

		Body* currentBase = Physics::GetBodyFromId(standingOnBody);

		// -------------------------------
		// 3) Platform attach/detach detection
		// -------------------------------
		bool wasAttached = (lastStandingOnBodyPtr != nullptr);
		bool isAttached = (currentBase != nullptr);

		if (wasAttached && !isAttached)
		{
			// Detached: apply last platform velocity to character
			// (use fresh computation for accuracy on rotation)
			vec3 basePos = FromPhysics(lastStandingOnBodyPtr->GetPosition());
			glm::quat baseRot = FromPhysics(lastStandingOnBodyPtr->GetRotation());
			vec3 linearVel = FromPhysics(lastStandingOnBodyPtr->GetLinearVelocity());
			vec3 angularVel = FromPhysics(lastStandingOnBodyPtr->GetAngularVelocity());
			vec3 worldOffset = baseRot * baseLocalAttachPoint;
			vec3 platformVelAtAttach = linearVel + glm::cross(angularVel, worldOffset);

			vec3 currentVel = GetVelocity();
			currentVel += platformVelAtAttach;
			SetVelocity(currentVel);
			lastPlatformVelocity = vec3(0.0f);
		}
		else if (wasAttached && isAttached && (currentBase != lastStandingOnBodyPtr))
		{
			// Switched platforms: detach from old, attach to new
			// Detach from old
			vec3 oldBasePos = FromPhysics(lastStandingOnBodyPtr->GetPosition());
			glm::quat oldBaseRot = FromPhysics(lastStandingOnBodyPtr->GetRotation());
			vec3 oldLinearVel = FromPhysics(lastStandingOnBodyPtr->GetLinearVelocity());
			vec3 oldAngularVel = FromPhysics(lastStandingOnBodyPtr->GetAngularVelocity());
			vec3 oldWorldOffset = oldBaseRot * baseLocalAttachPoint;
			vec3 oldPlatformVel = oldLinearVel + glm::cross(oldAngularVel, oldWorldOffset);

			vec3 currentVel = GetVelocity();
			currentVel += oldPlatformVel;
			SetVelocity(currentVel);
			lastPlatformVelocity = vec3(0.0f);  // Temporary reset

			// Attach to new (fall through to attach logic below)
			wasAttached = false;  // Force attach block to run
		}

		if (!wasAttached && isAttached)
		{
			// Attaching to a new platform: store local attach point
			vec3 basePos = FromPhysics(currentBase->GetPosition());
			glm::quat baseRot = FromPhysics(currentBase->GetRotation());
			vec3 charPos = FromPhysics(body->GetPosition());

			baseLocalAttachPoint = glm::inverse(baseRot) * (charPos - basePos);
			prevAttachWorldPos = charPos;
			prevBaseRotation = baseRot;
			prevBasePosition = basePos;

			// Adjust character's velocity to be relative to the platform
			vec3 linearVel = FromPhysics(currentBase->GetLinearVelocity());
			vec3 angularVel = FromPhysics(currentBase->GetAngularVelocity());
			vec3 worldOffset = baseRot * baseLocalAttachPoint;
			vec3 platformVelAtAttach = linearVel + glm::cross(angularVel, worldOffset);

			vec3 currentVel = GetVelocity();
			currentVel -= platformVelAtAttach;
			SetVelocity(currentVel);

			lastPlatformVelocity = platformVelAtAttach;
		}

		if (currentBase)
		{
			lastStandingOnBody = currentBase->GetID();
		}
		{
			lastStandingOnBody = BodyID();
		}


		// -------------------------------
		// 4) Original character movement / gravity / slope logic
		// -------------------------------
		vec3 velocity = GetVelocity();

		if (velocity.y > 0.1f)
		{
			onGround = false;
		}

		// Manual gravity — skipped when the ladder (or any future system) takes
		// sole ownership of vertical velocity via suppressGravity.
		if (!suppressGravity)
		{
			if (onGround && velocity.y < 0)
			{
				velocity.y -= gravity * deltaTime * (1.0f - walkableNormal.y);
			}
			else
			{
				velocity.y -= gravity * deltaTime;
			}
		}

		if (onGround && velocity.y <= 0)
		{
			vec3 currentPosition = FromPhysics(body->GetPosition());
			float newVerticalPosition = verticalPosition + stepHeight / 2.0f + height / 2.0f;

			float snapDelta = newVerticalPosition - currentPosition.y;
			if (std::abs(snapDelta) > 0.005f)
			{
				Physics::SweepBody(body, vec3(currentPosition.x, newVerticalPosition, currentPosition.z), { owner });

				float moved = currentPosition.y - FromPhysics(body->GetPosition()).y;
				if (std::abs(moved) > 0.001f)
				{
					heightSmoothOffset += moved;
				}
			}
		}

		if (onGround)
		{
			velocity.y = 0;
		}

		vec3 applyVelocity = velocity;

		if (onGround == false && standsOnGround && (velocity.y <= 0))
		{
			vec3 slopeNormal = normalize(notWalkableNormal);

			// 1. Calculate how much of our velocity is pointing INTO the slope
			float velocityIntoSlope = dot(velocity, slopeNormal);

			// 2. Only alter velocity if we are actually moving into the unwalkable surface
			if (velocityIntoSlope < 0)
			{
				// Clip the velocity. This removes the inward force and leaves 
				// ONLY the velocity running parallel down the slope.
				applyVelocity = velocity - slopeNormal * velocityIntoSlope;

				// Optional: Apply a slight over-bounce to prevent floating point penetration
				// applyVelocity = velocity - slopeNormal * (velocityIntoSlope * 1.001f);
			}
			else
			{
				// If moving away from the slope (e.g. jumping off), keep the trajectory
				applyVelocity = velocity;
			}

			UpdateSmoothPosition(deltaTime * 2);
		}
		else if (onGround == false && standsOnGround)
		{
			vec3 slopeNormal = normalize(notWalkableNormal);
			float velocityTowardSlope = dot(velocity, slopeNormal);
			if (velocityTowardSlope < 0)
			{
				vec3 slopeTangent = velocity - slopeNormal * velocityTowardSlope;
				float originalSpeed = length(velocity);

				applyVelocity = slopeTangent;
				applyVelocity.y = velocity.y;

				float newSpeed = length(applyVelocity);
				if (newSpeed > originalSpeed)
					applyVelocity = applyVelocity * (originalSpeed / newSpeed);

				UpdateSmoothPosition(deltaTime * 2);
			}
		}

		if (glm::isnan(applyVelocity).x || glm::isnan(applyVelocity).y || glm::isnan(applyVelocity).z)
		{
			assert(false);
			applyVelocity = vec3(0.01f);
		}

		// -------------------------------
		// 5) Set final velocity
		// -------------------------------
		SetVelocity(applyVelocity);

		// -------------------------------
		// 6) If still attached, update local attach point and platform velocity
		// -------------------------------
		if (lastStandingOnBodyPtr)
		{
			vec3 basePos = FromPhysics(lastStandingOnBodyPtr->GetPosition());
			glm::quat baseRot = FromPhysics(lastStandingOnBodyPtr->GetRotation());
			vec3 charPos = FromPhysics(body->GetPosition());

			baseLocalAttachPoint = glm::inverse(baseRot) * (charPos - basePos);
			prevAttachWorldPos = charPos;
			prevBaseRotation = baseRot;
			prevBasePosition = basePos;

			vec3 linearVel = FromPhysics(lastStandingOnBodyPtr->GetLinearVelocity());
			vec3 angularVel = FromPhysics(lastStandingOnBodyPtr->GetAngularVelocity());
			vec3 worldOffset = baseRot * baseLocalAttachPoint;
			lastPlatformVelocity = linearVel + glm::cross(angularVel, worldOffset);
		}
	}

	// -------------------------------
	// 7) Character-on-character stacking: ignore pairs + horizontal push
	// -------------------------------
	UpdateCharacterStacking(deltaTime);

	if (sensorBody && body)
	{
		Physics::SetBodyPosition(sensorBody, FromPhysics(body->GetPosition()) - vec3(0, stepHeight * 0.5f, 0));
		Physics::SetLinearVelocity(sensorBody, FromPhysics(body->GetLinearVelocity()));
	}


	// Update camera height target based on state
	targetCameraHeight = (isCrouched) ? cameraHeightCrouching : cameraHeightStanding;

	// Smooth camera height
	float smoothFactor = 1.0f - std::exp(-stepSmoothingSpeed * deltaTime);
	currentCameraHeight = glm::mix(currentCameraHeight, targetCameraHeight, smoothFactor);
}


//returns center of character controller
vec3 CharacterController::GetPosition()
{
	if (body)
	{
		return FromPhysics(body->GetPosition()) - vec3(0, stepHeight * 0.5f, 0);
	}

	return vec3();
}

vec3 CharacterController::GetSmoothPosition()
{
	return GetPosition() + vec3(0, heightSmoothOffset, 0);
}

//sets center of character controller
void CharacterController::SetPosition(vec3 position)
{
	if (body)
		Physics::SetBodyPosition(body, position + vec3(0, stepHeight * 0.5f, 0));
	if (sensorBody)
		Physics::SetBodyPosition(sensorBody, position + vec3(0, stepHeight * 0.5f, 0));
}

void CharacterController::SetSmoothPosition(vec3 position)
{
	if (body)
	{
		Physics::SetBodyPosition(body, position + vec3(0, stepHeight, 0) - vec3(0, heightSmoothOffset, 0));
	}
}

vec3 CharacterController::GetSmoothOffset()
{
	return vec3(0, heightSmoothOffset, 0);
}

void CharacterController::UpdateSmoothPosition(float deltaTime)
{

	heightSmoothOffset *= std::exp(-stepSmoothingSpeed * deltaTime);

}

vec3 CharacterController::GetVelocity()
{
	if (!body) return vec3();
	return FromPhysics(body->GetLinearVelocity());
}

void CharacterController::SetVelocity(vec3 vel)
{
	if (!body) return;
	Physics::SetLinearVelocity(body, vel);
}

float CharacterController::GetCameraHeight()
{
	return currentCameraHeight;
}

// ── Crouch ─────────────────────────────────────────────────────────────────
void CharacterController::Crouch()
{
	if (isCrouched) return;

	vec3 oldVel = GetVelocity();

	vec3 currentBodyPos = FromPhysics(body->GetPosition());
	float oldHeight = height;
	float newHeight = crouchHeight;
	float delta = oldHeight - newHeight;
	vec3 deltaPos(0.0f);

	if (onGround)
	{
		deltaPos.y = -delta / 2.0f;
	}
	else
	{
		float oldCam = cameraHeightStanding;
		float newCam = cameraHeightCrouching;
		deltaPos.y = (newHeight - oldHeight) * 0.5f + (oldCam - newCam);
		currentCameraHeight = cameraHeightCrouching;
	}

	vec3 newBodyPos = currentBodyPos + deltaPos;

	Destroy(); // destroys both body and sensorBody, cleans up ignore pairs

	stepHeight = 0.25f;

	body = Physics::CreateCharacterCylinderBody(owner, newBodyPos, radius, newHeight - stepHeight, 30);
	body->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(body)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(body)->sensorCollisionMode = SensorCollisionMode::NotCollideWithSensors;
	Physics::SetGravityFactor(body, 0);

	sensorBody = Physics::CreateCharacterCylinderBody(owner, newBodyPos, radius, newHeight - stepHeight, 30);
	sensorBody->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(sensorBody)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(sensorBody)->sensorCollisionMode = SensorCollisionMode::CollideOnlyWithSensors;
	Physics::SetGravityFactor(sensorBody, 0);

	height = newHeight;
	isCrouched = true;

	SetVelocity(oldVel);
}

// ── UnCrouch ───────────────────────────────────────────────────────────────
void CharacterController::UnCrouch()
{
	if (!isCrouched) return;
	if (!CanStandUp()) return;

	vec3 oldVel = GetVelocity();

	vec3 currentBodyPos = FromPhysics(body->GetPosition());
	float oldHeight = height;
	float newHeight = standingHeight;
	float delta = newHeight - oldHeight;
	vec3 deltaPos(0.0f);

	if (onGround)
	{
		deltaPos.y = delta / 2.0f;
	}
	else
	{
		float oldCam = cameraHeightCrouching;
		float newCam = cameraHeightStanding;
		deltaPos.y = (newHeight - oldHeight) * 0.5f + (oldCam - newCam);
		currentCameraHeight = cameraHeightStanding;
	}

	vec3 newBodyPos = currentBodyPos + deltaPos;

	Destroy(); // cleans up ignore pairs before destroying bodies

	stepHeight = 0.4f;

	body = Physics::CreateCharacterCylinderBody(owner, newBodyPos, radius, newHeight - stepHeight, 30);
	body->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(body)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(body)->sensorCollisionMode = SensorCollisionMode::NotCollideWithSensors;
	Physics::SetGravityFactor(body, 0);

	sensorBody = Physics::CreateCharacterCylinderBody(owner, newBodyPos, radius, newHeight - stepHeight, 30);
	sensorBody->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(sensorBody)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(sensorBody)->sensorCollisionMode = SensorCollisionMode::CollideOnlyWithSensors;
	Physics::SetGravityFactor(sensorBody, 0);

	height = newHeight;
	isCrouched = false;

	SetVelocity(oldVel);
}

bool CharacterController::CanStandUp()
{
	if (!isCrouched) return true;

	vec3 currentBodyPos = FromPhysics(body->GetPosition());
	float oldHeight = height;
	float newHeight = standingHeight;
	float delta = newHeight - oldHeight;
	vec3 deltaPos(0.0f);

	if (onGround)
	{
		deltaPos.y = delta / 2.0f;
	}
	else
	{
		deltaPos.y = -delta / 2.0f;
	}

	vec3 newBodyPos = currentBodyPos + deltaPos;
	float newPhysHalf = (newHeight - stepHeight * 2.0f) / 2.0f;
	vec3 bottom = newBodyPos;
	bottom -= vec3(0, 0.15f, 0);
	vec3 top = newBodyPos + vec3(0.0f, newPhysHalf, 0.0f);

	Physics::HitResult result = Physics::CylinderTrace(bottom, top, radius - 0.05f, 0.05f, BodyType::GroupCollisionTest, { body }, { owner }, true);


	return !result.hasHit;
}


float CharacterController::GroundAngleRad(const glm::vec3& normal)
{
	// ensure the normal is normalized
	glm::vec3 n = glm::normalize(normal);

	// dot with world up (0,1,0)
	float cosTheta = std::clamp(n.y, -1.0f, 1.0f);

	// angle between the two vectors
	return std::acos(cosTheta);
}

float CharacterController::GroundAngleDeg(const glm::vec3& normal)
{
	return glm::degrees(GroundAngleRad(normal));
}

// ── SetLadderMode ──────────────────────────────────────────────────────────
//
// Called by Player::EnterLadder / ExitLadder.
//
// What it does:
//   1. Toggles suppressGravity so CharacterController::Update() stops applying
//      manual gravity while the player is on a ladder.
//   2. Rebuilds the physics body with stepHeight = 0 on entry (restored on
//      exit).  Normally the capsule bottom sits 0.4 m above the player's feet
//      (the step-height gap).  On a ladder that raised bottom catches on the
//      top corner of the platform when descending.  Zeroing stepHeight makes
//      the capsule sit flush with the feet so it slides past cleanly.
//   3. Preserves logical position: GetPosition() returns the same value before
//      and after the call because bodyPos is derived from GetPosition() with
//      the NEW stepHeight baked in.
//
void CharacterController::SetLadderMode(bool enabled)
{
	if (isOnLadder == enabled) return;

	isOnLadder = enabled;
	suppressGravity = enabled;

	// Save/restore the pre-ladder velocity so the body rebuild is transparent.
	vec3 oldVel = GetVelocity();
	// Logical center — accounts for the current stepHeight offset so the
	// character doesn't jump when the body is replaced.
	vec3 logicalPos = GetPosition();

	if (enabled)
	{
		savedStepHeight = stepHeight;
		stepHeight = 0.0f;
	}
	else
	{
		stepHeight = savedStepHeight;
	}

	// Body center in physics space = logical center + stepHeight/2 up.
	// With stepHeight == 0 (ladder) body center IS the logical center.
	vec3 bodyPos = logicalPos + vec3(0.0f, stepHeight * 0.5f, 0.0f);

	Destroy(); // cleans up ignore pairs, nulls body & sensorBody

	// Main body — capsule, same construction as Init().
	body = Physics::CreateCharacterBody(owner, bodyPos, radius, standingHeight - stepHeight, 30);
	body->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(body)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(body)->sensorCollisionMode = SensorCollisionMode::NotCollideWithSensors;
	Physics::SetGravityFactor(body, 0);

	// Sensor body — cylinder, full standing height, centred on logical position.
	// Update() will keep re-snapping it to body->GetPosition() - stepHeight*0.5
	// every frame so the initial placement just needs to be in the right ballpark.
	sensorBody = Physics::CreateCharacterCylinderBody(owner, logicalPos, radius, standingHeight, 30);
	sensorBody->GetMotionProperties()->SetLinearDamping(0);
	Physics::GetBodyData(sensorBody)->dynamicCollisionGroupOrMask = true;
	Physics::GetBodyData(sensorBody)->sensorCollisionMode = SensorCollisionMode::CollideOnlyWithSensors;
	Physics::SetGravityFactor(sensorBody, 0);

	height = standingHeight;

	SetVelocity(oldVel);
}

void CharacterController::UpdateGroundCheck(bool& hitsGround, float& calculatedGroundHeight, bool& canStand, vec3& avgNormal, vec3& notWalkableNormal)
{
	hitsGround = false;
	calculatedGroundHeight = 0;
	avgNormal = vec3(0, 1, 0);
	canStand = false;
	standingOnBody = BodyID();
	standingOnEntity = EntityHandle();
	notWalkableNormal = vec3(0, 1, 0);

	vec3 heightOffset = vec3(0, stepHeight, 0);
	float outheight = 0;
	bool outCanStand = false;
	vec3 outNormal = vec3();
	const Body* hitBody = nullptr;
	Entity* hitEntity = nullptr;

	// -------------------------------------------------------
	// NPC fast path: single center cast, skip ring samples
	// if surface is flat (normal.y close to 1.0)
	// -------------------------------------------------------
	if (movementQuality == CharacterControllerMovementQuality::NpcGeneric)
	{
		constexpr float flatThreshold = 0.99f; // ~cos(4.4 degrees)

		bool centerHit = CheckGroundAt(
			FromPhysics(body->GetPosition()) - heightOffset,
			radius - 0.01f,
			outheight, outCanStand, outNormal, &hitBody, &hitEntity);

		if (centerHit && (outNormal.y >= flatThreshold || GetVelocity().x != 0 || GetVelocity().z != 0))
		{
			// Perfectly flat ground - no need to sample around ring
			avgNormal = outNormal;
			float heightComp = GetPosition().y - height / 2.0f - 0.001f;
			hitsGround = (outheight > heightComp);
			canStand = hitsGround && outCanStand;
			calculatedGroundHeight = outheight;

			if (outCanStand)
			{
				standingOnBody = hitBody->GetID();
				standingOnEntity = EntityHandle::FromEntity(hitEntity);
			}

			return;
		}

		// Sloped or missed — fall through to reduced NPC sample loop below
	}


	if (movementQuality == CharacterControllerMovementQuality::NpcLowQuality)
	{
		constexpr float flatThreshold = MathHelper::constexpr_cos(30.0f * M_PI / 180.0f); // ~cos(30 degrees)

		bool centerHit = CheckGroundAt(
			FromPhysics(body->GetPosition()) - heightOffset,
			0,
			outheight, outCanStand, outNormal, &hitBody, &hitEntity);

		if (centerHit && outNormal.y >= flatThreshold)
		{
			// Perfectly flat ground - no need to sample around ring
			avgNormal = outNormal;
			float heightComp = GetPosition().y - height / 2.0f - 0.001f;
			hitsGround = (outheight > heightComp);
			canStand = hitsGround && outCanStand;
			calculatedGroundHeight = outheight;

			if (outCanStand)
			{
				standingOnBody = hitBody->GetID();
				standingOnEntity = EntityHandle::FromEntity(hitEntity);
			}

			return;
		}

		// Sloped or missed — fall through to reduced NPC sample loop below
	}

	// -------------------------------------------------------
	// Full / reduced sample loop
	// Walkable and non-walkable hits are accumulated separately.
	// Non-walkable hits (steep surfaces) still detect that ground
	// exists (hitsGround) but do NOT poison avgNormal or height.
	// If no walkable hits were found at all, we fall back to the
	// full set so behaviour on fully-steep ground is unchanged.
	// -------------------------------------------------------
	int numOfIterations = 16;
	float startRadius = 0.1f;
	float rayRadius = 0.1f;
	float radiusStep = 0.3f;

	if (ThreadPool::Supported() == false)
	{
		startRadius = 0.405f;
		numOfIterations = 4;
	}

	if (movementQuality == CharacterControllerMovementQuality::Player)
	{
		numOfIterations = 8;
		startRadius = 0.25f;
		rayRadius = 0.0f;
		radiusStep = 0.25f;
	}

	if (movementQuality == CharacterControllerMovementQuality::NpcGeneric)
	{
		numOfIterations = 4;
		startRadius = 1;
		rayRadius = 0;

		if (length(GetVelocity()) > 0.2f)
			startRadius = 0.5f;

	}

	if (movementQuality == CharacterControllerMovementQuality::NpcLowQuality)
	{
		numOfIterations = 4;
		startRadius = 1;
		rayRadius = 0;

		if (length(GetVelocity()) > 0.2f)
			startRadius = 1.0f;

	}

	// --- walkable bucket ---
	vec3  walkNormalSum = vec3(0);
	float walkHeightSum = 0;
	int   walkHits = 0;
	int   walkNormalCount = 0;

	// --- non-walkable bucket (steep / bad surfaces) ---
	vec3  steepNormalSum = vec3(0);
	float steepHeightSum = 0;
	int   steepHits = 0;
	int   steepNormalCount = 0;

	const float heightComp = GetPosition().y - height / 2.0f - 0.001f;

	// Helper: route one CheckGroundAt result into the right bucket.
	// Weight is 1 for ring samples, 3 for the weighted center cast.
	auto AccumulateHit = [&](bool hit, float weight, const Body* hb, Entity* he)
		{
			if (!hit) return;

			bool walkable = outCanStand; // set by the last CheckGroundAt call

			if (outheight > heightComp)
				hitsGround = true;

			if (walkable)
			{
				walkNormalSum += outNormal;
				walkHeightSum += outheight * weight;
				walkHits += static_cast<int>(weight);
				walkNormalCount += 1;
				canStand = true;
				if (hb) standingOnBody = hb->GetID();
				if (he) standingOnEntity = EntityHandle::FromEntity(he);
			}
			else
			{
				steepNormalSum += outNormal;
				steepHeightSum += outheight * weight;
				steepHits += static_cast<int>(weight);
				steepNormalCount += 1;
			}
		};

	// Ring samples
	for (float r = startRadius; r <= 1; r += radiusStep)
	{

		if (movementQuality == CharacterControllerMovementQuality::Player)
		{
			numOfIterations = 4
				+ 4 * (r >= 0.5f)
				+ 8 * (r >= 0.8f);
		}

		for (int i = 0; i < numOfIterations; i++)
		{
			float angle = (2.0f * M_PI / numOfIterations) * i;
			vec3  offset = vec3(cos(angle), 0.0f, sin(angle)) * (radius * r - rayRadius - 0.05f);

			bool hit = CheckGroundAt(
				FromPhysics(body->GetPosition()) + offset - heightOffset,
				rayRadius, outheight, outCanStand, outNormal, &hitBody, &hitEntity);

			AccumulateHit(hit, 1.0f, hitBody, hitEntity);
		}
	}

	// Weighted center cast (weight 3 — same as before)
	{
		bool hit = CheckGroundAt(
			FromPhysics(body->GetPosition()) - heightOffset,
			radius - 0.01f, outheight, outCanStand, outNormal, &hitBody, &hitEntity);

		AccumulateHit(hit, 3.0f, hitBody, hitEntity);
	}

	// -------------------------------------------------------
	// Resolve: prefer walkable bucket; fall back to full set
	// only when there are zero walkable hits (pure steep ground).
	// -------------------------------------------------------
	if (walkHits > 0)
	{
		// Normal surface or mixed (player partially on steep edge):
		// use only walkable samples so steep patches don't tilt the
		// normal and cause the bogus downhill acceleration / slow-down.
		avgNormal = walkNormalSum / static_cast<float>(walkNormalCount);
		calculatedGroundHeight = walkHeightSum / static_cast<float>(walkHits);
	}
	else if (steepHits > 0)
	{
		// Entirely on steep ground — keep old behaviour so gravity /
		// slope-slide still work correctly.
		avgNormal = steepNormalSum / static_cast<float>(steepNormalCount);
		calculatedGroundHeight = steepHeightSum / static_cast<float>(steepHits);
	}

	if (steepNormalCount > 0)
	{
		notWalkableNormal = steepNormalSum / static_cast<float>(steepNormalCount);
	}
	else
	{
		notWalkableNormal = vec3(0, 1, 0);
	}

	const int totalHits = walkHits + steepHits;
	hitsGround = hitsGround && (totalHits > 0);
	canStand = hitsGround && (GroundAngleDeg(avgNormal) <= groundMaxAngle) && canStand && walkHits > 3;
}

bool CharacterController::CheckGroundAt(vec3 location, float checkRadius, float& outheight, bool& canStand, vec3& normal, const Body** hitBody, Entity** hitEntity)
{
	Physics::HitResult result;

	vec3 start = location + vec3(0, 0.2f, 0);
	vec3 end = location - vec3(0, height / 2.0f + stepHeight, 0);

	if (checkRadius > 0)
	{
		result = Physics::CylinderTrace(start, end - vec3(0, 0.05f, 0), checkRadius, 0.05f, BodyType::GroupCharacter, { body, sensorBody }, { owner }, true);
		result.position = result.shapePosition - vec3(0, 0.02f, 0);
	}
	else
	{
		result = Physics::LineTrace(start, end, BodyType::GroupCharacter, { body, sensorBody }, { owner }, true);
	}

	*hitBody = result.hitbody;
	if (hitEntity) *hitEntity = result.entity;

	if (result.normal.y < 0.1)
		return false;

	// Discard hits against any other character's bodies entirely.
	// Returning false means the hit goes into no bucket — standsOnGround
	// stays false and the slope-clip code in Update() never strips vertical
	// velocity, which was what was holding the character up.
	for (const CharacterController* other : s_allControllers)
	{
		if (other == this) continue;
		if (result.hitbody == other->body || result.hitbody == other->sensorBody)
			return false;
	}

	outheight = result.position.y;

	canStand = result.hasHit && (GroundAngleDeg(result.normal) <= groundMaxAngle);

	normal = result.normal;

	return result.hasHit;
}

// ---------------------------------------------------------------------------
// UpdateCharacterStacking
//
// Called once per Update().  This controller is the "top" character.
//
// 1. Scan s_allControllers to find characters that are currently below us
//    (our feet near their top, XZ overlap).
// 2. Add ignore pairs for newly detected bodies so we pass through them
//    vertically while still being able to push horizontally.
// 3. Remove ignore pairs for bodies that are no longer below us.
// 4. Apply a symmetric horizontal separation impulse to both bodies for
//    every active stacking pair so they push apart over time.
// ---------------------------------------------------------------------------
void CharacterController::UpdateCharacterStacking(float deltaTime)
{
	if (!body) return;

	const BodyID myID = body->GetID();
	const vec3   myPos = GetPosition();
	const float  myFeetY = myPos.y - height * 0.5f;

	// ── 1. Detect which characters are currently below us ───────────────────
	std::vector<BodyID> detectedThisFrame;

	for (CharacterController* other : s_allControllers)
	{
		if (other == this || !other->body) continue;

		const vec3  otherPos = other->GetPosition();
		const float otherTopY = otherPos.y + other->height * 0.5f;
		const float combinedRadii = radius + other->radius;

		// Horizontal distance (XZ only)
		const vec3  diff3 = myPos - otherPos;
		const float xzDist = glm::length(vec2(diff3.x, diff3.z));

		// yDiff ~= 0 when our feet sit exactly on their top.
		// Accept a window above (+height*0.5) so that the pair is detected
		// slightly before contact, and below (-stepHeight - 0.3) to keep
		// the pair alive while we are passing through.
		const float yDiff = myFeetY - otherTopY;
		const bool  verticallyStacked = (yDiff > -(stepHeight + 0.3f)) && (yDiff < height * 0.5f);

		// Small extra margin so the pair is established just before the
		// bodies visually overlap.
		const bool  horizontallyClose = xzDist < combinedRadii + 0.15f;

		if (verticallyStacked && horizontallyClose)
			detectedThisFrame.push_back(other->body->GetID());
	}

	// ── 2. Add ignore pairs for newly detected bodies ────────────────────────
	for (const BodyID& id : detectedThisFrame)
	{
		const bool alreadyIgnored = std::find(activeIgnorePairs.begin(), activeIgnorePairs.end(), id) != activeIgnorePairs.end();
		if (!alreadyIgnored)
		{
			Physics::AddIgnorePair(myID, id);
			activeIgnorePairs.push_back(id);
			removeCollisionCooldown[id].AddDelay(0.3);
		}
	}

	// ── 3. Remove ignore pairs for bodies no longer below us ─────────────────
	for (auto it = activeIgnorePairs.begin(); it != activeIgnorePairs.end(); )
	{
		bool stillDetected = std::find(detectedThisFrame.begin(), detectedThisFrame.end(), *it) != detectedThisFrame.end();
		if (removeCollisionCooldown[*it].Wait())
			stillDetected = true;

		if (!stillDetected)
		{
			Physics::RemoveIgnorePair(myID, *it);
			it = activeIgnorePairs.erase(it);
		}
		else
			++it;
	}

	// ── 4. Horizontal separation push ────────────────────────────────────────
	// Only the top character (this) drives the push so there is no
	// double-counting between two controllers that are stacked.
	for (CharacterController* other : s_allControllers)
	{
		if (other == this || !other->body) continue;

		const bool isPaired = std::find(activeIgnorePairs.begin(), activeIgnorePairs.end(), other->body->GetID()) != activeIgnorePairs.end();
		if (!isPaired) continue;

		vec3 diff = GetPosition() - other->GetPosition();
		diff.y = 0.0f; // horizontal only

		const float dist = glm::length(diff);
		const float combinedRadii = radius + other->radius;
		const float overlap = combinedRadii - dist;

		if (overlap <= 0.0f) continue;

		// Degenerate case: characters at the exact same XZ position.
		// Pick an arbitrary direction so they always separate.
		const vec3 pushDir = (dist > 0.001f)
			? glm::normalize(diff)
			: vec3(1.0f, 0.0f, 0.0f);

		// Frame-rate-independent continuous push, scaled by overlap depth.
		const float magnitude = overlap * 8.0f;
		Physics::AddImpulse(body, pushDir * (magnitude * deltaTime));
		Physics::AddImpulse(other->body, pushDir * (-magnitude * deltaTime));
	}
}