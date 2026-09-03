// NpcNight.cpp
#include "NpcHumanBase.h"

class NpcNight : public NpcHumanBase
{
private:
	bool attackDamageDealt = false;

	static constexpr float AttackDamageDelay = 0.85f;
	static constexpr float AttackEndThreshold = 0.15f;

	void UpdateAttackDamage();
	void UpdateAttackState();

protected:
	void Attack()     override;
	void LoadAssets() override;

public:
	NpcNight();

	void Stun() override;
	void OnDamage(float Damage, Entity* DamageCauser = nullptr,
		Entity* Weapon = nullptr) override;
	void AsyncUpdate() override;
	void Serialize(json& target) override;
	void Deserialize(json& source) override;
	void NetSerialize(NetPacket& packet) override;
	void NetDeserialize(NetPacket& packet) override;
};

NpcNight::NpcNight()
{
	ClassName = "npc_knight";
	maxSpeed = 5.5f;
	canBeStunRagdolled = false;
	Health = 170;
	MaxHealth = 170;
	canHear = true;
}

void NpcNight::Stun()
{
	NpcHumanBase::Stun();
	attackDamageDealt = false;
}

// Attack – same logic runs on all peers; owner enforces cooldown.
void NpcNight::Attack()
{
	if (isOwned)
	{
		if (inAttackDelay.Wait()) return;
		inAttackDelay.AddDelay(1.5f);
	}

	state = NpcState::Attacking;
	attackDamageDealt = false;
	PlaySoundEffect("event:/NPC/Enemy1/Enemy1AttackStart");
	mesh->PlayAnimation("attack", false, 0.3f);
	mesh->PullRootMotion();

	if (isOwned)
	{
		NetPacket args(PacketType::RPC);
		SendRPC(static_cast<uint8_t>(NpcRPC::Attack), args, RPCTarget::Others);
	}
}

void NpcNight::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
	NpcHumanBase::OnDamage(Damage, DamageCauser, Weapon);
}

// Hit detection – owner only; calls OnPointDamage which broadcasts to Others.
void NpcNight::UpdateAttackDamage()
{
	if (!isOwned)          return;
	if (attackDamageDealt) return;
	if (mesh->GetAnimationTime() < AttackDamageDelay) return;

	auto hit = Physics::SphereTrace(
		Position,
		MathHelper::GetForwardVector(mesh->Rotation) * 1.7f + Position,
		0.55f,
		BodyType::World | BodyType::CharacterCapsule,
		{ controller.body });

	if (!hit.hasHit || !hit.entity->HasTag("player")) return;

	attackDamageDealt = true;

	if (NpcHelper::CheckParry(MathHelper::GetForwardVector(mesh->Rotation), hit.entity))
	{
		OnPointDamage(20, hit.shapePosition,
			MathHelper::FastNormalize(Position - hit.shapePosition),
			"", this, this);
		Physics::AddImpulse(mesh->FindHitboxByName("spine_02"),
			MathHelper::GetForwardVector(mesh->Rotation) * -500.0f);
		AddDebuffStacks("PoiseBreakDebuff", 100);
	}
	else
	{
		hit.entity->OnPointDamage(20, hit.shapePosition,
			MathHelper::FastNormalize(hit.shapePosition - Position),
			"", this, this);
	}
}

void NpcNight::UpdateAttackState()
{
	float dur = mesh->GetAnimationDuration();
	if (dur > 0.0f && mesh->GetAnimationTime() >= dur - AttackEndThreshold)
	{
		state = NpcState::Chasing;
		attackDamageDealt = false;
		mesh->PlayAnimation("run", true, 0.5f);
	}
}

void NpcNight::AsyncUpdate()
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

	mesh->Position = Position - vec3(0, 1, 0) + controller.GetSmoothOffset();

	// Root motion applies on ALL peers – zombie only uses it during stun.
	auto rootMotion = mesh->PullRootMotion();
	if (IsStunned() || state == NpcState::Attacking)
	{
		Position += rootMotion.Position;
		controller.SetPosition(Position);
		if (rootMotion.Position != vec3())
			controller.SetVelocity(vec3(0, controller.GetVelocity().y, 0));
		if (rootMotion.Rotation != vec3())
		{
			mesh->Rotation += rootMotion.Rotation;
			movingDirection = MathHelper::GetForwardVector(mesh->Rotation);
		}
	}

	UpdateStunnedReturn();
	UpdateReturnFromRagdoll();

	// Time-based stun end (zombie rig has no stun_end event).
	if (IsStunned() && !stunnedRagdoll && !returningFromRagdoll)
	{
		float dur = mesh->GetAnimationDuration();
		if (dur > 0.0f && mesh->GetAnimationTime() >= dur - AttackEndThreshold)
		{
			state = NpcState::Idle;
			mesh->PlayAnimation("run", true, 0.5f);
		}
	}

	mesh->UpdateHitboxes();

	if (soundPlayer)
	{
		soundPlayer->Position = vec3(mesh->GetBoneMatrixWorld("head")[3]);
		soundPlayer->Velocity = controller.GetVelocity();
	}

	if (IsDead() || IsStunned() || stunnedRagdoll || returningFromRagdoll) return;


	if (IsAttacking())
	{
		UpdateAttackDamage();
		if (IsDead()) return;
		if (IsAttacking()) UpdateAttackState();
		if (IsAttacking())
		{
			vec3 vel = controller.GetVelocity();
			controller.SetVelocity(vec3(0, vel.y, 0));
			return;
		}
	}

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

	vec3 lookAtDir = MathHelper::FastNormalize(resolvedTarget->Position - Position);

	if (glm::distance(resolvedTarget->Position, Position) < 1.5f &&
		glm::dot(MathHelper::GetForwardVector(mesh->Rotation), lookAtDir) > 0.93f)
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

	if (controller.onGround)
	{
		controller.SetVelocity(vec3(movingDirection.x * speed, vel.y, movingDirection.z * speed));
	}

	mesh->Rotation = vec3(0,
		MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);

}

void NpcNight::NetSerialize(NetPacket& packet)
{
	NpcHumanBase::NetSerialize(packet);
	packet.WriteBool(attackDamageDealt);
}

void NpcNight::NetDeserialize(NetPacket& packet)
{
	NpcHumanBase::NetDeserialize(packet);
	bool remoteAttackDamageDealt = packet.ReadBool();
	if (!isOwned)
		attackDamageDealt = remoteAttackDamageDealt;
}

void NpcNight::Serialize(json& target)
{
	NpcHumanBase::Serialize(target);
	SERIALIZE_FIELD(target, attackDamageDealt);
}

void NpcNight::Deserialize(json& source)
{
	NpcHumanBase::Deserialize(source);
	DESERIALIZE_FIELD(source, attackDamageDealt);
}

void NpcNight::LoadAssets()
{
	NpcHumanBase::LoadAssets();
	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");
	mesh->TexturesLocation = "GameData/models/enemies/knight/knight.glb/";
	mesh->LoadFromFile("GameData/models/enemies/knight/knight.glb");
	mesh->PreloadAssets();
	mesh->CreateHitboxes(this);
	mesh->PlayAnimation("run", true);
	mesh->SetLooped(true);
}

REGISTER_ENTITY(NpcNight, "npc_knight")
