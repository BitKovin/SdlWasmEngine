// NpcHumanGun.cpp
#include "NpcHumanGun.h"
#include "../../Player/Player.hpp"
#include "../../Player/Weapons/Projectiles/Bullet.h"
#include <RandomHelper.h>

REGISTER_ENTITY(NpcHumanGun, "npc_human_gun")

NpcHumanGun::NpcHumanGun()
{
	ClassName = "npc_human_gun";
	maxSpeed = 4.2f;
	mesh->Scale = vec3(1.15f);
	Health = 80;
	MaxHealth = 80;
	pathFollow.allowPartialPath = true;
}

void NpcHumanGun::ProcessAnimationEvent(AnimationEvent& event)
{
	if (event.eventName == "stun_end")
	{
		stuned = false;
		mesh->PlayAnimation("run", true, 0.5f);
	}
}

void NpcHumanGun::Attack()
{
	if (cantAttackDelay.Wait()) return;
	if (inAttackDelay.Wait()) return;

	if (target == nullptr) return;

	vec3 bonePos = mesh->GetBoneMatrixWorld("weapon_muzzle")[3];

	vec3 predictedTargetPosition = target->Position;

	if (AttackDirectionCheck(bonePos, predictedTargetPosition, target) == false)
	{
		inAttackDelay.AddDelay(0.5f);
		return;
	}

	const float bulletSpeed = 50;

	Player* playerRef = dynamic_cast<Player*>(target);

	if (playerRef)
	{
		predictedTargetPosition += playerRef->controller.GetVelocity() * distance(Position, target->Position) / bulletSpeed;
	}
	else
	{
		if (target->LeadBody)
		{
			predictedTargetPosition += FromPhysics(target->LeadBody->GetLinearVelocity()) * distance(Position, target->Position) / bulletSpeed;
		}
	}

	predictedTargetPosition += RandomHelper::RandomPosition(distance(target->Position, Position) / 20.0f) * (accuracyModifier + 1.0f);

	vec3 bulletRotation = MathHelper::FindLookAtRotation(bonePos, predictedTargetPosition);

	Bullet* bullet = (Bullet*)Spawn("bullet");
	bullet->LoadAssets();
	bullet->Position = bonePos;
	bullet->Rotation = bulletRotation;
	bullet->Speed = bulletSpeed;
	bullet->OwnerTag = "enemy";
	bullet->Damage = 5;
	bullet->owner = this;
	bullet->Start();

	PlaySoundEffect("event:/NPC/Enemy1/Enemy1AttackStart");

	inAttackDelay.AddDelay(ModifyAnimationSpeed(0.5f));
	afterAttackDelay.AddDelay(3);
	mesh->PlayAnimation("fire");
	mesh->PullRootMotion();

	// Shoot-and-scoot: count the shot; reposition once the burst is done.
	// Skip repositioning if already at the closest reachable navmesh position.
	shotsFired++;
	if (shotsFired >= shotsPerBurst)
	{
		shotsFired = 0;
		shotsPerBurst = 0;
		if (!pathFollow.reachedTarget)
		{
			repositioning = true;
			repositionTarget = FindAttackLocation();
			cantAttackDelay.AddDelay(0.5f);
		}
	}
}

bool NpcHumanGun::LineOfSightCheck(Entity* targetEntity)
{
	auto hit = Physics::SphereTrace(Position + vec3(0, 0.4f, 0), targetEntity->Position + vec3(0, 0.3f, 0), 0.4,
		BodyType::World,
		{  }, { this, targetEntity });

	return hit.hasHit == false;
}

bool NpcHumanGun::AttackDirectionCheck(vec3 start, vec3 target, Entity* targetEntity)
{
	auto hit = Physics::SphereTrace(start, target, 0.2,
		BodyType::GroupHitTest,
		{  }, { this, targetEntity });

	return hit.hasHit == false;
}

void NpcHumanGun::AsyncUpdate()
{
	if (dead)
	{
		mesh->UpdateHitboxes();

		UpdateStatusWidgets();

		return;
	}

	controller.Update(Time::DeltaTimeF);
	Position = controller.GetPosition();

	UpdatePerception();

	accuracyModifier -= Time::DeltaTimeF / 3.0f;
	accuracyModifier = glm::clamp(accuracyModifier, 0.0f, 5.0f);

	UpdateStatusWidgets();

	UpdateDebuffs(Time::DeltaTimeF);

	if (dead) return;

	mesh->Update(ModifyAnimationSpeed(1.0f));

	auto animEvents = mesh->PullAnimationEvents();

	for (auto& event : animEvents)
	{
		ProcessAnimationEvent(event);
	}

	mesh->Position = Position - vec3(0, 1, 0);

	auto rootMotion = mesh->PullRootMotion();

	Position += rootMotion.Position;
	controller.SetPosition(Position);
	if (rootMotion.Position != vec3())
		controller.SetVelocity(vec3(0, controller.GetVelocity().y, 0));

	if (rootMotion.Rotation != vec3())
	{
		mesh->Rotation += rootMotion.Rotation;
		movingDirection = MathHelper::GetForwardVector(mesh->Rotation);
	}

	UpdateStunnedReturn();
	UpdateReturnFromRagdoll();

	mesh->UpdateHitboxes();

	soundPlayer->Position = vec3(mesh->GetBoneMatrixWorld("head")[3]);

	soundPlayer->Velocity = controller.GetVelocity();

	if (dead || stuned || stunnedRagdoll || returningFromRagdoll) return;

	// No target: idle in place
	if (target == nullptr)
	{
		if (mesh->GetAnimationName() != "aim")
			mesh->PlayAnimation("aim", true, 0.5f);

		afterAttackDelay.AddDelay(2);

		vec3 vel = controller.GetVelocity();
		controller.SetVelocity(vec3(0, vel.y, 0));
		return;
	}

	auto animName = mesh->GetAnimationName();

	if (animName == "run" || animName == "stun")
	{
		cantAttackDelay.AddDelay(0.7f);
	}

	desiredTargetLocation = target->Position;

	vec3 lookAtDir = MathHelper::FastNormalize(target->Position - Position);

	bool hasLineOfSight = LineOfSightCheck(target);

	float attackDistance = 34;

	if (distance2(target->Position, Position) > attackDistance * attackDistance)
	{
		hasLineOfSight = false;
	}

	// ── Shoot-and-scoot state machine ──────────────────────────────────────────

	if (repositioning)
	{
		// REPOSITIONING phase: sprint to the chosen flank position
		if (animName != "run" && !stuned)
			mesh->PlayAnimation("run", true, 0.6f);

		cantAttackDelay.AddDelay(0.5f);

		if (pathFollow.reachedTarget)
		{
			// Arrived at destination – switch to shooting phase
			repositioning = false;
			shotsPerBurst = 3 + (RandomHelper::RandomInt() % 2);
			shotsFired = 0;

			if (animName == "run")
				mesh->PlayAnimation("aim", true, 0.3f);
		}
		else
		{
			desiredTargetLocation = repositionTarget;
		}
	}
	else
	{
		// SHOOTING phase: stand still and fire the burst

		// Roll burst size on first entry
		if (shotsPerBurst == 0)
		{
			shotsPerBurst = 2 + (RandomHelper::RandomInt() % 3);
			shotsFired = 0;
		}

		if (hasLineOfSight)
			afterAttackDelay.AddDelay(3);

		if (hasLineOfSight || afterAttackDelay.Wait())
		{
			if (animName == "run")
				mesh->PlayAnimation("aim", true, 0.3f);
			else
				desiredDirection = lookAtDir;

			if (cantAttackDelay.Wait() == false)
				Attack();
		}
		else
		{
			// Lost LOS mid-burst. Only reposition if there is somewhere to go;
			// if reachedTarget is true we are already at the closest navmesh
			// point – repositioning would just loop back here immediately.
			if (!pathFollow.reachedTarget)
			{
				if (animName != "run" && !stuned)
					mesh->PlayAnimation("run", true, 0.6f);

				cantAttackDelay.AddDelay(0.5f);
				repositioning = true;
				repositionTarget = FindAttackLocation();
				shotsFired = 0;
				shotsPerBurst = 0;
			}
			else
			{
				// Stuck at closest possible position with no LOS – hold aim and wait.
				if (animName == "run")
					mesh->PlayAnimation("aim", true, 0.3f);
			}
		}
	}

	// ── End state machine ──────────────────────────────────────────────────────

	if (fleeing)
	{
		UpdateFleeTarget();
	}
	else
	{
		pathFollow.WaitToFinish();
		pathFollow.UpdateStartAndTarget(Position, desiredTargetLocation);
		pathFollow.TryPerform();
	}

	if (pathFollow.FoundTarget && animName == "run")
	{
		desiredDirection = normalize(MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position));
	}

	speed += Time::DeltaTimeF * 6.5;

	speed = glm::clamp(speed, 0.0f, ModifyMovementSpeed(maxSpeed));

	if ((hasLineOfSight || afterAttackDelay.Wait()) && !repositioning)
	{
		speed = 0;
	}

	movingDirection = mix(movingDirection, desiredDirection, Time::DeltaTime * 10);

	movingDirection = MathHelper::FastNormalize(movingDirection);

	vec3 vel = controller.GetVelocity();
	controller.SetVelocity(vec3(movingDirection.x * speed, vel.y, movingDirection.z * speed));

	mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);
}

void NpcHumanGun::Serialize(json& target)
{
	NpcHumanBase::Serialize(target);

	SERIALIZE_FIELD(target, cantAttackDelay);
	SERIALIZE_FIELD(target, desiredTargetLocation);
	SERIALIZE_FIELD(target, accuracyModifier);
	SERIALIZE_FIELD(target, repositioning);
	SERIALIZE_FIELD(target, repositionTarget);
	SERIALIZE_FIELD(target, shotsFired);
	SERIALIZE_FIELD(target, shotsPerBurst);
}

void NpcHumanGun::Deserialize(json& source)
{
	NpcHumanBase::Deserialize(source);

	DESERIALIZE_FIELD(source, cantAttackDelay);
	DESERIALIZE_FIELD(source, desiredTargetLocation);
	DESERIALIZE_FIELD(source, accuracyModifier);
	DESERIALIZE_FIELD(source, repositioning);
	DESERIALIZE_FIELD(source, repositionTarget);
	DESERIALIZE_FIELD(source, shotsFired);
	DESERIALIZE_FIELD(source, shotsPerBurst);
}

void NpcHumanGun::LoadAssets()
{
	NpcHumanBase::LoadAssets();

	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

	mesh->TexturesLocation = "GameData/models/enemies/humanGun/humanGun.glb/";
	mesh->LoadFromFile("GameData/models/enemies/humanGun/humanGun.glb");
	mesh->PreloadAssets();
	mesh->CreateHitboxes(this);
	mesh->PlayAnimation("run", true);
	mesh->SetLooped(true);
}

vec3 NpcHumanGun::FindAttackLocation()
{
	// Detection parameters
	const float tooCloseThreshold = 2.0f;

	// Candidate generation parameters
	const int maxIterations = 100;
	const int normalTargetCandidates = 20;
	const int tooCloseTargetCandidates = 30;
	const float preferredMaxMove = 5.0f;
	const float absoluteMaxMove = 8.0f;

	// Ring multipliers
	struct Ring {
		float minMult;
		float maxMult;
	};
	const std::vector<Ring> rings = {
		{0.85f, 1.15f}, // Primary: optimal range
		{0.65f, 0.85f}, // Secondary: closer
		{0.40f, 0.65f}  // Tertiary: evasive close
	};

	// Normal mode angle biases (probabilities cumulative)
	const float normalLeftFlankProb = 0.35f;
	const float normalRightFlankProb = 0.35f;
	const float normalBackProb = 0.20f;
	const float normalFrontProb = 0.10f; // Remaining
	const float normalLeftFlankAngleBase = -90.0f;
	const float normalLeftFlankAngleVar = 80.0f;
	const float normalRightFlankAngleBase = 90.0f;
	const float normalRightFlankAngleVar = 80.0f;
	const float normalBackAngleBase = 180.0f;
	const float normalBackAngleVar = 120.0f;
	const float normalFrontAngleBase = 0.0f;
	const float normalFrontAngleVar = 40.0f;

	// Too close mode angle biases (probabilities cumulative)
	const float tooCloseDirectAwayProb = 0.60f;
	const float tooCloseLeftFlankProb = 0.20f;
	const float tooCloseRightFlankProb = 0.20f; // Remaining
	const float tooCloseDirectAwayAngleBase = 0.0f;
	const float tooCloseDirectAwayAngleVar = 20.0f;
	const float tooCloseLeftFlankAngleBase = 60.0f;
	const float tooCloseLeftFlankAngleVar = 60.0f;
	const float tooCloseRightFlankAngleBase = -60.0f;
	const float tooCloseRightFlankAngleVar = 60.0f;

	// Path checking parameters
	const float segmentDivisor = 2.5f;
	const int maxSegments = 10;

	// Fallback parameters
	const float fallbackDist = 4.0f;
	const float fallbackDistMult = 0.8f;

	// Scoring parameters - common
	const float distSigma = 0.15f;
	const float randomScoreMult = 0.25f;
	const float moveScoreWeight = 1.8f;
	const float circleScoreWeight = 1.1f;

	// Scoring parameters - normal
	const float normalMovePeak = 0.3f;
	const float normalMoveSigma = 0.25f;
	const float normalFlankWeight = 3.2f;
	const float normalDistWeight = 2.1f;

	// Scoring parameters - too close
	const float tooCloseMovePeak = 0.7f;
	const float tooCloseMoveSigma = 0.35f;
	const float tooCloseFlankWeight = 1.8f;
	const float tooCloseDistWeight = 3.2f;

	// Clustering penalty parameters
	const float clusterSigma = 2.0f;
	const float clusterWeight = 2.5f;

	// Scatter parameters to break perfect circle
	const float scatterRadiusMult = 0.15f;

	const vec3& currentPos = Position;
	auto targetEntity = target;
	const vec3& targetPos = targetEntity->Position;
	const vec3& targetRot = targetEntity->Rotation;
	vec3 targetForward = MathHelper::GetForwardVector(targetRot);
	targetForward.y = 0.0f;
	targetForward = MathHelper::Normalized(targetForward);
	vec3 targetRight = MathHelper::Normalized(glm::cross(targetForward, vec3(0.0f, 1.0f, 0.0f)));

	float currentDistXZ = glm::length(MathHelper::XZ(currentPos - targetPos));
	bool isTooClose = currentDistXZ < tooCloseThreshold;

	vec3 referenceForward = targetForward;
	vec3 referenceRight = targetRight;

	int targetCandidatesLocal = normalTargetCandidates;
	if (isTooClose) {
		referenceForward = MathHelper::Normalized(MathHelper::XZ(currentPos - targetPos));
		referenceRight = MathHelper::Normalized(glm::cross(referenceForward, vec3(0.0f, 1.0f, 0.0f)));
		targetCandidatesLocal = tooCloseTargetCandidates;
	}

	// Collect positions of other NPCs targeting the same target
	std::vector<vec3> otherNpcPositions;

	std::vector<vec3> candidates;

	int iter = 0;
	const float desired = attackDesiredRange;

	bool relaxedMoveConstraint = false;
	for (const auto& ring : rings) {
		while (candidates.size() < static_cast<size_t>(targetCandidatesLocal) && iter < maxIterations) {
			++iter;

			// Biased relative angle generation (target-relative flanking or evade)
			float r = RandomHelper::RandomFloat();
			float relAngleDeg;
			if (isTooClose) {
				// Evade: 60% direct away, 20% left flank away, 20% right flank away
				if (r < tooCloseDirectAwayProb) {
					relAngleDeg = tooCloseDirectAwayAngleBase + (RandomHelper::RandomFloat() - 0.5f) * tooCloseDirectAwayAngleVar;
				}
				else if (r < tooCloseDirectAwayProb + tooCloseLeftFlankProb) {
					relAngleDeg = tooCloseLeftFlankAngleBase + (RandomHelper::RandomFloat() - 0.5f) * tooCloseLeftFlankAngleVar;
				}
				else {
					relAngleDeg = tooCloseRightFlankAngleBase + (RandomHelper::RandomFloat() - 0.5f) * tooCloseRightFlankAngleVar;
				}
			}
			else {
				if (r < normalLeftFlankProb) { // 35% left flank
					relAngleDeg = normalLeftFlankAngleBase + (RandomHelper::RandomFloat() - 0.5f) * normalLeftFlankAngleVar;
				}
				else if (r < normalLeftFlankProb + normalRightFlankProb) { // 35% right flank
					relAngleDeg = normalRightFlankAngleBase + (RandomHelper::RandomFloat() - 0.5f) * normalRightFlankAngleVar;
				}
				else if (r < normalLeftFlankProb + normalRightFlankProb + normalBackProb) { // 20% back
					relAngleDeg = normalBackAngleBase + (RandomHelper::RandomFloat() - 0.5f) * normalBackAngleVar;
				}
				else { // 10% front
					relAngleDeg = normalFrontAngleBase + (RandomHelper::RandomFloat() - 0.5f) * normalFrontAngleVar;
				}
			}

			float relAngleRad = MathHelper::ToRadians(relAngleDeg);
			vec3 dirToPos = MathHelper::Normalized(
				std::cos(relAngleRad) * referenceForward +
				std::sin(relAngleRad) * referenceRight
			);

			float distMult = ring.minMult + RandomHelper::RandomFloat() * (ring.maxMult - ring.minMult);
			float dist = desired * distMult;
			dist = std::min(dist, desired); // Ensure <= desiredRange for CheckAttackLocation
			vec3 newPos = targetPos + dirToPos * dist;

			// Add scatter to break perfect circle
			float scatterRadius = desired * scatterRadiusMult;
			vec3 scatter = RandomHelper::RandomPosition(scatterRadius);
			scatter.y = 0.0f;
			newPos += scatter;
			if (glm::length(MathHelper::XZ(newPos - targetPos)) > desired) continue;

			// Check movement distance constraint (prefer close, relax if needed)
			float moveDist = glm::distance(newPos, currentPos);
			float maxMove = relaxedMoveConstraint ? absoluteMaxMove : preferredMaxMove;
			if (moveDist > maxMove) continue;

			// Direct path check
			auto pathHit = Physics::CylinderTrace(currentPos, newPos, 0.5f, 0.8f, BodyType::World | BodyType::MainBody);
			if (pathHit.hasHit) continue;

			// Intermediate LOS checks on subdivided path
			bool pathLOSClear = true;
			vec3 diff = newPos - currentPos;
			float pathLen = glm::length(diff);
			int segments = 1 + static_cast<int>(pathLen / segmentDivisor);
			segments = std::min(maxSegments, segments);
			for (int s = 1; s < segments; ++s) {
				float t = static_cast<float>(s) / static_cast<float>(segments);
				vec3 midPos = currentPos + diff * t;
				if (!CheckAttackLocation(midPos, targetPos)) {
					pathLOSClear = false;
					break;
				}
			}
			if (!pathLOSClear) continue;

			// Final LOS check
			if (!CheckAttackLocation(newPos, targetPos)) continue;

			candidates.push_back(newPos);
		}

		// If not enough candidates after a ring, relax move constraint for subsequent rings
		if (candidates.size() < static_cast<size_t>(targetCandidatesLocal / 2)) {
			relaxedMoveConstraint = true;
		}
		if (candidates.size() >= static_cast<size_t>(targetCandidatesLocal / 2)) break;
	}

	// Ultimate fallback
	if (candidates.empty()) {
		if (isTooClose) {
			// Perpendicular escape when too close
			vec3 radialDir = MathHelper::Normalized(MathHelper::XZ(currentPos - targetPos));
			vec3 perpRight = MathHelper::Normalized(glm::cross(radialDir, vec3(0.0f, 1.0f, 0.0f)));
			vec3 fallbackDirs[2] = { perpRight, -perpRight };
			float fallbackDistLocal = std::min(fallbackDist, desired * fallbackDistMult);
			for (int i = 0; i < 2; ++i) {
				vec3 fallbackPos = currentPos + fallbackDirs[i] * fallbackDistLocal;
				auto h = Physics::CylinderTrace(currentPos, fallbackPos, 0.5f, 0.8f, BodyType::World | BodyType::MainBody);
				if (!h.hasHit && CheckAttackLocation(fallbackPos, targetPos)) {
					return fallbackPos;
				}
			}
			return currentPos; // Stay if no escape
		}
		else {
			return targetPos;
		}
	}

	// Advanced scoring system
	vec3 bestPos = candidates[0];
	float bestScore = -std::numeric_limits<float>::max();

	vec3 currRelDir = MathHelper::Normalized(MathHelper::XZ(currentPos - targetPos));

	float movePeak = isTooClose ? tooCloseMovePeak : normalMovePeak;
	float moveSigma = isTooClose ? tooCloseMoveSigma : normalMoveSigma;
	float flankWeight = isTooClose ? tooCloseFlankWeight : normalFlankWeight;
	float distWeight = isTooClose ? tooCloseDistWeight : normalDistWeight;

	for (const vec3& cand : candidates) {
		vec3 toTarget = MathHelper::XZ(cand - targetPos);
		float cDist = glm::length(toTarget);
		vec3 attackDir = (cDist > 0.001f) ? toTarget / cDist : vec3(0.0f, 0.0f, 1.0f);

		// Tactical alignment score: flank vs target or away alignment (higher when aligned/opposite)
		float tacticalScore = -glm::dot(attackDir, referenceForward);

		// Distance score: Gaussian peak at desired range
		float distDiff = (cDist - desired) / desired;
		float distScore = std::exp(-distDiff * distDiff / (2.0f * distSigma * distSigma));

		// Movement score: Gaussian favoring appropriate movement distance
		float moveDistScore = glm::distance(cand, currentPos);
		float moveNorm = moveDistScore / desired;
		float moveScore = std::exp(-(moveNorm - movePeak) * (moveNorm - movePeak) / (2.0f * moveSigma * moveSigma));

		// Circling score: prefer tangential movement
		vec3 moveDir = MathHelper::Normalized(MathHelper::XZ(cand - currentPos));
		float circleScore = 1.0f - std::abs(glm::dot(moveDir, currRelDir));

		// Randomness for unpredictability
		float randScore = RandomHelper::RandomFloat() * randomScoreMult;

		// Clustering penalty: penalize proximity to other NPCs' current positions
		float clusterPenalty = 0.0f;
		for (const vec3& otherPos : otherNpcPositions) {
			float d = glm::distance(cand, otherPos);
			if (d < 0.001f) continue;
			clusterPenalty += std::exp(-(d * d) / (2.0f * clusterSigma * clusterSigma));
		}

		// Weighted total score
		float score = tacticalScore * flankWeight +
			distScore * distWeight +
			moveScore * moveScoreWeight +
			circleScore * circleScoreWeight +
			randScore -
			clusterPenalty * clusterWeight;

		if (score > bestScore) {
			bestScore = score;
			bestPos = cand;
		}
	}

	return bestPos;
}


bool NpcHumanGun::CheckAttackLOS(vec3 location, vec3 targetLocation)
{
	if (distance(location, targetLocation) < 1.5f) return true;

	vec3 attackDir = normalize(targetLocation - location) * 1.0f;

	vec3 attackPos = location + vec3(0, 0.6f, 0) + attackDir;

	auto hit = Physics::SphereTrace(attackPos, targetLocation + vec3(0, 0.65, 0), 0.1f, BodyType::GroupHitTest, mesh->hitboxBodies);

	if (hit.hasHit && hit.entity != target)
	{
		return false;
	}

	return true;
}

bool NpcHumanGun::CheckAttackLocation(vec3 location, vec3 targetLocation)
{
	if (distance(location, targetLocation) > attackDesiredRange)
	{
		return false;
	}

	if (CheckAttackLOS(location, targetLocation))
		return true;

	return true;
}