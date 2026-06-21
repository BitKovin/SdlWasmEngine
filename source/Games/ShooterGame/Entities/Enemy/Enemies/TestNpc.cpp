// TestNpc.cpp
#include "TestNpc.hpp"

//REGISTER_ENTITY(TestNpc, "testnpc")
REGISTER_ENTITY(TestNpc, "npc_dog")

TestNpc::TestNpc()
{
 	ClassName = "testnpc";
	maxSpeed  = 6.5f;
	Health    = 70;
	MaxHealth = 70;

	// Dog rig is a quadruped: it has no calf_l/calf_r/thigh_l/thigh_r
	// hitboxes, so the base class's leg-hit ragdoll trigger doesn't apply.
	canBeStunRagdolled = false;

	mesh->MeshCustomShaderParams["rim_pow"] = vec4(2.0f);

	deathSoundPath = "event:/NPC/Dog/DogDeath";
	stunSoundPath = "event:/NPC/Dog/DogStun";
	damageSoundPath = "";

}

// ---------------------------------------------------------------------------
// Animation events – run on ALL peers (no isOwned gate).
// ---------------------------------------------------------------------------

void TestNpc::ProcessAnimationEvent(AnimationEvent& event)
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

void TestNpc::Stun()
{
	NpcHumanBase::Stun();
	attackingDamage = false;
}

// ---------------------------------------------------------------------------
// Attack
//
// Same owner-decides / broadcast-to-others pattern as NpcHumanAxe: the owner
// enforces the cooldown and calls Attack() locally, then RPCs Others so every
// peer plays the identical bite animation. Hit detection stays owner-only.
// ---------------------------------------------------------------------------

void TestNpc::Attack()
{
	if (isOwned)
	{
		if (inAttackDelay.Wait()) return;
		inAttackDelay.AddDelay(1.5f);
	}

	state = NpcState::Attacking;
	PlaySoundEffect("event:/NPC/Dog/DogAttackStart");
	mesh->PlayAnimation("attack");
	mesh->PullRootMotion();

	if (isOwned)
	{
		NetPacket args(PacketType::RPC);
		SendRPC(static_cast<uint8_t>(NpcRPC::Attack), args, RPCTarget::Others);
	}
}

// ---------------------------------------------------------------------------
// Damage
//
// Below 30 health the dog breaks off and flees – but only from an
// interruptible state, so a dog that's mid-bite, already fleeing, or
// stunned isn't yanked out of what it's doing.
// ---------------------------------------------------------------------------

void TestNpc::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{ 
	NpcHumanBase::OnDamage(Damage, DamageCauser, Weapon);

	if (!IsDead() && Health < 30.0f && (IsIdle() || IsChasing()))
		state = NpcState::Fleeing;
}

// ---------------------------------------------------------------------------
// Hit detection – owner only.
// Calls OnPointDamage / Stun, which run effects locally and broadcast to
// Others through the inherited networking.
// ---------------------------------------------------------------------------

void TestNpc::UpdateAttackDamage()
{
	if (!isOwned)         return;
	if (!attackingDamage) return;

	auto hit = Physics::SphereTrace(
		Position,
		MathHelper::GetForwardVector(mesh->Rotation) * 0.75f + Position,
		0.2f,
		BodyType::World | BodyType::CharacterCapsule,
		{}, { this });

	if (!hit.hasHit) return;
	if (!hit.entity->HasTag("player")) return;

	if (NpcHelper::CheckParry(MathHelper::GetForwardVector(mesh->Rotation), hit.entity))
	{

		if (Health > 20)
		{
			OnPointDamage(15, hit.shapePosition,
				MathHelper::FastNormalize(Position - hit.shapePosition),
				"", this, this);
		}


		AddDebuffStacks("PoiseBreakDebuff", 100);
		Stun();
		attackingDamage = false;
		return;
	}

	hit.entity->OnPointDamage(ModifyOutgoingDamage(10), hit.shapePosition,
		MathHelper::FastNormalize(hit.shapePosition - Position), "", this, this);
	attackingDamage = false;

	PlaySoundEffect("event:/NPC/Dog/DogAttack");
}

// ---------------------------------------------------------------------------
// Main update
// ---------------------------------------------------------------------------

void TestNpc::AsyncUpdate()
{
	if (IsDead())
	{
		mesh->UpdateHitboxes();
		UpdateStatusWidgets();
		return;
	}

	controller.Update(Time::DeltaTimeF);
	Position = controller.GetPosition();

	ResolveTarget();

	UpdatePerception();

	UpdateStatusWidgets();
	UpdateDebuffs(Time::DeltaTimeF);

	if (IsDead()) return;

	mesh->Update(ModifyAnimationSpeed(1.0f));

	auto animEvents = mesh->PullAnimationEvents();
	for (auto& ev : animEvents)
		ProcessAnimationEvent(ev);  // runs on all peers

	mesh->Position = Position - vec3(0, 1, 0);

	// Root motion applies on ALL peers.
	// The owner's position is authoritative; non-owners are corrected each
	// frame by the snapshot lerp in NetDeserialize, so local root motion
	// makes movement smooth instead of snappy.
	auto rootMotion = mesh->PullRootMotion();

	if (length(rootMotion.Position) > 0.03)
	{

		auto hit = Physics::SphereTrace(Position, Position + rootMotion.Position, 0.2, BodyType::GroupCollisionTest, {}, { this });

		if (hit.hasHit)
		{

			Position = hit.shapePosition + hit.normal * 0.2f;
			controller.SetPosition(Position);
		}
		else
		{
			Position += rootMotion.Position;
			controller.SetPosition(Position);
		}

	}
	else
	{
		Position += rootMotion.Position;
		controller.SetPosition(Position);
	}

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

	if (glm::distance(calculatedTargetPos, Position) < 5.0f &&
		glm::dot(MathHelper::GetForwardVector(mesh->Rotation), lookAtDir) > 0.975f)
	{
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

	speed += Time::DeltaTimeF * 6.5f;
	speed = glm::clamp(speed, 0.0f, ModifyMovementSpeed(maxSpeed));

	movingDirection = glm::mix(movingDirection, desiredDirection,
		(double)Time::DeltaTime * 10.0);
	movingDirection = MathHelper::FastNormalize(movingDirection);

	vec3 vel = controller.GetVelocity();
	controller.SetVelocity(vec3(movingDirection.x * speed, vel.y,
		movingDirection.z * speed));
	mesh->Rotation = vec3(0,
		MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);
}

// ---------------------------------------------------------------------------
// Replication
// ---------------------------------------------------------------------------

void TestNpc::NetSerialize(NetPacket& packet)
{
	NpcHumanBase::NetSerialize(packet);
	packet.WriteBool(attackingDamage);
}

void TestNpc::NetDeserialize(NetPacket& packet)
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

void TestNpc::Serialize(json& target)
{
	NpcHumanBase::Serialize(target);
	SERIALIZE_FIELD(target, attackingDamage);
}

void TestNpc::Deserialize(json& source)
{
	NpcHumanBase::Deserialize(source);
	DESERIALIZE_FIELD(source, attackingDamage);
}

// ---------------------------------------------------------------------------
// Asset loading
// ---------------------------------------------------------------------------

void TestNpc::LoadAssets()
{
	NpcHumanBase::LoadAssets();
	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");
	mesh->LoadFromFile("GameData/models/enemies/dog/dog.glb");
	mesh->PreloadAssets();
	mesh->CreateHitboxes(this);
	mesh->PlayAnimation("run", true);
	mesh->SetLooped(true);
	mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");
	mesh->CustomId = 1;
}
