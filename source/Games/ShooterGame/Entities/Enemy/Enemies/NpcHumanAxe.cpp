// NpcHumanAxe.cpp
#include "NpcHumanAxe.h"

REGISTER_ENTITY(NpcHumanAxe, "npc_human_axe")

NpcHumanAxe::NpcHumanAxe()
{
	ClassName = "npc_human_axe";
	maxSpeed = 7.0f;
	pathFollow.allowPartialPath = true;
	Health = 160;
	MaxHealth = 160;
}

// ---------------------------------------------------------------------------
// Animation events – run on ALL peers (no isOwned gate).
// ---------------------------------------------------------------------------

void NpcHumanAxe::ProcessAnimationEvent(AnimationEvent& event)
{
	if (event.eventName == "attack_start")
	{
		attackingDamage = true;
	}

	if (event.eventName == "attack_end")
	{
		mesh->PlayAnimation("run", true, 0.5f);
		state = NpcState::Chasing;
		attackingDamage = false;
	}

	if (event.eventName == "stun_end")
	{
		state = NpcState::Idle;
		mesh->PlayAnimation("run", true, 0.5f);
	}
}

// ---------------------------------------------------------------------------
// Stun – delegates to base which broadcasts to Others.
// ---------------------------------------------------------------------------

void NpcHumanAxe::Stun()
{
	NpcHumanBase::Stun();
	attackingDamage = false;
}

// ---------------------------------------------------------------------------
// Attack
//
// The owner decides WHEN to attack (cooldown + range check).
// Once the decision is made, it calls Attack() locally and broadcasts via RPC
// to RPCTarget::Others so every other peer calls Attack() too.
// The actual animation, sound and state change run identically everywhere.
// Hit detection (UpdateAttackDamage) stays owner-only since it is the
// authoritative collision check – it calls OnPointDamage which broadcasts
// to Others.
// ---------------------------------------------------------------------------

void NpcHumanAxe::Attack()
{
	// Owner enforces cooldown; non-owners just play the animation when the
	// RPC arrives (cooldown has already been checked by the owner).
	if (isOwned)
	{
		if (inAttackDelay.Wait()) return;
		inAttackDelay.AddDelay(1.5f);
	}

	state = NpcState::Attacking;
	PlaySoundEffect("event:/NPC/Enemy1/Enemy1AttackStart");
	mesh->PlayAnimation("attack");
	mesh->PullRootMotion();

	// Broadcast to every other peer so they play the same animation.
	if (isOwned)
	{
		NetPacket args(PacketType::RPC);
		SendRPC(static_cast<uint8_t>(NpcRPC::Attack), args, RPCTarget::Others);
	}
}

// ---------------------------------------------------------------------------
// Damage
// ---------------------------------------------------------------------------

void NpcHumanAxe::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
	NpcHumanBase::OnDamage(Damage, DamageCauser, Weapon);
}

// ---------------------------------------------------------------------------
// Hit detection – owner only.
// Calls OnPointDamage which runs effects locally and broadcasts to Others.
// ---------------------------------------------------------------------------

void NpcHumanAxe::UpdateAttackDamage()
{
	if (!isOwned)         return;
	if (!attackingDamage) return;

	auto hit = Physics::SphereTrace(
		Position,
		MathHelper::GetForwardVector(mesh->Rotation) * 1.6f + Position,
		0.45f,
		BodyType::World | BodyType::CharacterCapsule,
		{}, { this });

	if (!hit.hasHit) return;

	if (hit.entity->HasTag("player"))
	{
		if (NpcHelper::CheckParry(MathHelper::GetForwardVector(mesh->Rotation), hit.entity))
		{

			if (Health > 25)
			{
				OnPointDamage(20, hit.shapePosition,
					MathHelper::FastNormalize(Position - hit.shapePosition),
					"", this, this);
				Physics::AddImpulse(mesh->FindHitboxByName("spine_02"),
					MathHelper::GetForwardVector(mesh->Rotation) * -500.0f);
			}

			AddDebuffStacks("PoiseBreakDebuff", 100);
			attackingDamage = false;
			return;
		}
		else
		{
			hit.entity->OnPointDamage(15, hit.shapePosition,
				MathHelper::FastNormalize(hit.shapePosition - Position),
				"", this, this);
			attackingDamage = false;
		}
	}
}

// ---------------------------------------------------------------------------
// Main update
// ---------------------------------------------------------------------------

void NpcHumanAxe::AsyncUpdate()
{
	if (IsDead())
	{
		mesh->UpdateHitboxes();
		UpdateStatusWidgets();
		return;
	}

	controller.Update(Time::DeltaTimeF);
	Position = controller.GetPosition();
	speed = glm::length(MathHelper::XZ(controller.GetVelocity()));

	ResolveTarget();

	UpdatePerception();

	UpdateStatusWidgets();
	UpdateDebuffs(Time::DeltaTimeF);

	if (IsDead()) return;

	mesh->Update(ModifyAnimationSpeed(1.0f));

	auto animEvents = mesh->PullAnimationEvents();
	for (auto& ev : animEvents)
		ProcessAnimationEvent(ev);  // runs on all peers

	mesh->Position = Position - vec3(0, 1, 0) + controller.GetSmoothOffset();

	// Root motion applies on ALL peers.
	// The owner's position is authoritative; non-owners are corrected each
	// frame by the snapshot lerp in NetDeserialize, so local root motion
	// makes movement smooth instead of snappy.
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

	if (soundPlayer)
	{
		soundPlayer->Position = vec3(mesh->GetBoneMatrixWorld("head")[3]);
		soundPlayer->Velocity = controller.GetVelocity();
	}

	if (IsDead() || IsStunned() || stunnedRagdoll || returningFromRagdoll) return;


	// Hit detection is authoritative; only the owner runs it.
	UpdateAttackDamage();
	if (IsDead()) return;

	if (resolvedTarget == nullptr)
	{
		if (mesh->GetAnimationName() != "idle")
			mesh->PlayAnimation("idle", true, 0.5f);
		state = NpcState::Idle;
		vec3 vel = controller.GetVelocity();
		controller.SetVelocity(vec3(0, vel.y, 0));
		return;
	}

	if (mesh->GetAnimationName() == "idle")
		mesh->PlayAnimation("run", true, 0.5f);
	if (state == NpcState::Idle)
		state = NpcState::Chasing;

	if (IsAttacking())
	{
		speed = 2.0f;
		return;
	}

	vec3 calculatedTargetPos = resolvedTarget->Position;
	if (resolvedTarget->Position.y < Position.y - 0.1f &&
		resolvedTarget->Position.y > Position.y - 1.5f)
		calculatedTargetPos.y = Position.y;

	vec3 lookAtDir = MathHelper::FastNormalize(calculatedTargetPos - Position);

	if (glm::distance(calculatedTargetPos, Position) < 1.5f &&
		glm::dot(MathHelper::GetForwardVector(mesh->Rotation), lookAtDir) > 0.93f)
	{
		if(isOwned)
		Attack();
	}

	if (IsFleeing())
		UpdateFleeTarget();
	else
	{
		pathFollow.UpdateStartAndTarget(Position, resolvedTarget->Position);
		pathFollow.TryPerform();
	}

	if (pathFollow.FoundTarget)
	{
		desiredDirection = glm::normalize(
			MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position));
	}
	else
	{
		// Path not ready or target unreachable – walk directly toward target
		// so the NPC never falls back to a stale or zero desiredDirection.
		vec3 direct = MathHelper::XZ(resolvedTarget->Position - Position);
		float len = glm::length(direct);
		if (len > 0.001f)
			desiredDirection = direct / len;
	}

	speed += Time::DeltaTimeF * 13.5f;
	speed = glm::clamp(speed, 0.0f, ModifyMovementSpeed(maxSpeed));

	movingDirection = glm::mix(movingDirection, desiredDirection,
		(double)Time::DeltaTime * 10.0);
	movingDirection = MathHelper::FastNormalize(movingDirection);

	vec3 vel = controller.GetVelocity();

	if (controller.onGround)
	{
		controller.SetVelocity(vec3(movingDirection.x * speed, vel.y, movingDirection.z * speed));
	}

	mesh->Rotation = vec3(0,
		MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);


}

// ---------------------------------------------------------------------------
// Replication
// ---------------------------------------------------------------------------

void NpcHumanAxe::NetSerialize(NetPacket& packet)
{
	NpcHumanBase::NetSerialize(packet);
	packet.WriteBool(attackingDamage);
}

void NpcHumanAxe::NetDeserialize(NetPacket& packet)
{
	NpcHumanBase::NetDeserialize(packet);
	// Always read to keep the cursor aligned; only apply on non-owner.
	bool remoteAttackingDamage = packet.ReadBool();
	if (!isOwned)
		attackingDamage = remoteAttackingDamage;
}

// ---------------------------------------------------------------------------
// Save-game serialization
// ---------------------------------------------------------------------------

void NpcHumanAxe::Serialize(json& target)
{
	NpcHumanBase::Serialize(target);
	SERIALIZE_FIELD(target, attackingDamage);
}

void NpcHumanAxe::Deserialize(json& source)
{
	NpcHumanBase::Deserialize(source);
	DESERIALIZE_FIELD(source, attackingDamage);
}

// ---------------------------------------------------------------------------
// Asset loading
// ---------------------------------------------------------------------------

void NpcHumanAxe::LoadAssets()
{
	NpcHumanBase::LoadAssets();
	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");
	mesh->TexturesLocation = "GameData/models/enemies/humanAxe/humanAxe.glb/";
	mesh->LoadFromFile("GameData/models/enemies/humanAxe/humanAxe.glb");
	mesh->PreloadAssets();
	mesh->CreateHitboxes(this);
	mesh->PlayAnimation("run", true);
	mesh->SetLooped(true);
}
