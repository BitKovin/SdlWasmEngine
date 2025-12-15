#include "TestNpc.hpp"
#include "Player/Player.hpp"

#include <Particle/GlobalParticleSystem.hpp>
#include <SoundSystem/FmodEventInstance.h>
#include <Navigation/Navigation.hpp>
#include "Npc/NpcHelper.h"

REGISTER_ENTITY(TestNpc, "testnpc")

void TestNpc::UpdateFleeTarget()
{

	if (fleeSearchDelay.Wait() == false)
	{

		Entity* target = Player::Instance;

		auto path = NavigationSystem::FindFleePath(Position, target->Position);

		if (path.empty() == false)
		{

			pathFollow.UpdateStartAndTarget(Position, path[path.size() - 1]);
			pathFollow.TryPerform();

		}

		fleeSearchDelay.AddDelay(0.2f);
	}

}

void TestNpc::ProcessAnimationEvent(AnimationEvent& event)
{
	
	Logger::Log(event.eventName);

	if (event.eventName == "attack_start")
	{
		attackingDamage = true;
	}

	if (event.eventName == "attack_end")
	{
		mesh->PlayAnimation("run", true, 0.5f);
		attacking = false;
		attackingDamage = false;
	}
	if (event.eventName == "stun_end")
	{
		stuned = false;
		mesh->PlayAnimation("run", true, 0.5f);
	}

}

void TestNpc::Start()
{

	mesh->Position = Position - vec3(0, 1, 0);
	mesh->Rotation = Rotation;

	//Drawables.push_back(mesh);

	LeadBody = Physics::CreateCharacterBody(this, Position, 0.5, 2, 50);

	Logger::Log("started npc\n");

	Physics::SetGravityFactor(LeadBody, 4);

	desiredDirection = MathHelper::XZ(MathHelper::GetForwardVector(Rotation));
	movingDirection = desiredDirection;

	pathFollow.CalculatePathOnThread();

	DeathSoundPlayer = SoundPlayer::Create();
	HurtSoundPlayer = SoundPlayer::Create();
	StunSoundPlayer = SoundPlayer::Create();
	AttackSoundPlayer = SoundPlayer::Create();
	AttackHitSoundPlayer = SoundPlayer::Create();

	SetupSoundPlayer(DeathSoundPlayer);
	DeathSoundPlayer->MaxDistance *= 1.5f;
	DeathSoundPlayer->Volume *= 2.0f;
	SetupSoundPlayer(HurtSoundPlayer);
	HurtSoundPlayer->Volume *= 1.5f;
	SetupSoundPlayer(StunSoundPlayer);
	SetupSoundPlayer(AttackSoundPlayer);
	AttackSoundPlayer->Volume *= 0.7f;
	SetupSoundPlayer(AttackHitSoundPlayer);
	AttackHitSoundPlayer->Volume *= 1.2;
}

void TestNpc::Stun(Entity* DamageCauser, Entity* Weapon)
{
	stuned = true;
	mesh->PlayAnimation("stun");
	mesh->PullRootMotion();
	attacking = false;
	attackingDamage = false;

	

	StunSoundPlayer->Play();

}

void TestNpc::Attack()
{

	AttackSoundPlayer->Play();

	inAttackDelay.AddDelay(2.5f);
	mesh->PlayAnimation("attack");
	mesh->PullRootMotion();
	attacking = true;

}

void TestNpc::Death()
{

	if (dead) return;

	//mesh->ClearHitboxes();
	mesh->StartRagdoll();
	mesh->SetAnimationPaused(true);
	Physics::SetLinearVelocity(LeadBody, vec3(0));

	DeathSoundPlayer->Play();

	//Physics::SetBodyType(LeadBody, BodyType::None);
	//Physics::SetCollisionMask(LeadBody, BodyType::World);

	Physics::DestroyBody(LeadBody);
	LeadBody = nullptr;

	CallActionOnEntityWithId(OwnerId, "despawned");

	dead = true;

	if (DeathSoundPlayer)
	{
		DeathSoundPlayer->DestroyWithDelay(3);
		HurtSoundPlayer->DestroyWithDelay(3);
		StunSoundPlayer->DestroyWithDelay(3);
		AttackSoundPlayer->DestroyWithDelay(3);
		AttackHitSoundPlayer->DestroyWithDelay(3);
		DeathSoundPlayer = nullptr;
		HurtSoundPlayer = nullptr;
		StunSoundPlayer = nullptr;
		AttackSoundPlayer = nullptr;
	}

	//Tags.clear();

}

void TestNpc::OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon)
{
	Damage *= mesh->GetHitboxDamageMultiplier(bone);
	Entity::OnPointDamage(Damage, Point, Direction, bone, DamageCauser, Weapon);

	if (mesh->GetHitboxDamageMultiplier(bone) > 1.1f && dead)
	{
		Time::AddTimeScaleEffect(0.3f, 0.15f, true, "hit_slow");
	}

	GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point, MathHelper::FindLookAtRotation(vec3(0), Direction), vec3(Damage/10.0f));

}

void TestNpc::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{

	Health -= Damage;

	if (Health <= 0)
	{
		Death();
	}
	else if(attacking)
	{
		Stun(DamageCauser, Weapon);
	}

	if (LeadBody)
	{
		LeadBody->SetLinearVelocity(LeadBody->GetLinearVelocity() / 2.0f);
		speed /= 2.0f;
		HurtSoundPlayer->Play();
	}

	if (Health < 30)
	{
		fleeing = true;
		UpdateFleeTarget();
	}

}

void TestNpc::UpdateAttackDamage()
{

	if (attackingDamage == false) return;

	auto hit = Physics::SphereTrace(Position, MathHelper::GetForwardVector(mesh->Rotation)*0.75f + Position, 0.2f, BodyType::World | BodyType::CharacterCapsule, { LeadBody });

	if (hit.hasHit)
	{
		if (hit.entity->HasTag("player"))
		{

			if (NpcHelper::CheckParry(MathHelper::GetForwardVector(mesh->Rotation), hit.entity))
			{
				Stun(hit.entity, hit.entity);
				OnPointDamage(10, hit.shapePosition, MathHelper::FastNormalize(Position - hit.shapePosition), "", this, this);
				attackingDamage = false;
				return;
			}
			else
			{
				hit.entity->OnPointDamage(20, hit.shapePosition, MathHelper::FastNormalize(hit.shapePosition - Position), "", this, this);
				attackingDamage = false;


				AttackHitSoundPlayer->Play();
			}

		}
	}

}

void TestNpc::AsyncUpdate()
{


	//mesh->UpdatePose = mesh->WasRended;

	mesh->Update();
	

	auto animEvents = mesh->PullAnimationEvents();

	for (auto& event : animEvents)
	{
		ProcessAnimationEvent(event);
	}

	mesh->Position = Position - vec3(0, 1, 0);



	auto rootMotion = mesh->PullRootMotion();

	if (LeadBody != nullptr)
	{
		Physics::MoveBody(LeadBody, rootMotion.Position);

		if (rootMotion.Position != vec3())
		{
			Physics::SetLinearVelocity(LeadBody, vec3(0, LeadBody->GetLinearVelocity().GetY(), 0));
		}

	}
	else
	{
		Position += rootMotion.Position;
	}
	
	if (rootMotion.Rotation != vec3())
	{
		mesh->Rotation += rootMotion.Rotation;
		movingDirection = MathHelper::GetForwardVector(mesh->Rotation);
	}


	mesh->UpdateHitboxes();

	if (dead || stuned) return;

	UpdateAttackDamage();

	if (dead || stuned) return;

	DeathSoundPlayer->Position = Position;
	HurtSoundPlayer->Position = Position;
	StunSoundPlayer->Position = Position;
	AttackSoundPlayer->Position = Position;
	AttackHitSoundPlayer->Position = Position;

	DeathSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());
	HurtSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());
	StunSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());
	AttackSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());
	AttackHitSoundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());

	Entity* target = Player::Instance;

	if (attacking)
	{


		return;
	}

	vec3 lookAtDir = MathHelper::FastNormalize(target->Position - Position);

	if (distance(target->Position, Position) < 5 
		&& dot(MathHelper::GetForwardVector(mesh->Rotation), lookAtDir) > 0.9)
	{
		
		Attack();
	}

	if (target)
	{
		if (fleeing)
		{
			UpdateFleeTarget();
		}
		else
		{

			pathFollow.UpdateStartAndTarget(Position, target->Position);
			pathFollow.TryPerform();

		}

	}
	if (pathFollow.FoundTarget)
	{
		desiredDirection = normalize(MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position));
	}
	
	speed += Time::DeltaTimeF * 6.5;

	speed = glm::clamp(speed, 0.0f, maxSpeed);

	movingDirection = mix(movingDirection, desiredDirection, Time::DeltaTime*5);

	movingDirection = MathHelper::FastNormalize(movingDirection);

	// Get the current horizontal velocity (preserving the vertical component from physics)
	vec3 currentVelocity = FromPhysics(LeadBody->GetLinearVelocity());
	vec3 currentHorizontalVel(currentVelocity.x, 0.0f, currentVelocity.z);

	// Determine the desired horizontal velocity (5.0f is the intended speed)
	vec3 desiredHorizontalVel = movingDirection * speed;

	// Calculate the change in velocity you need to achieve over the current frame
	// Using Time::DeltaTime (dt) to convert velocity difference to the required acceleration
	float dt = Time::DeltaTime;
	vec3 neededAcceleration = (desiredHorizontalVel - currentHorizontalVel) / dt;

	// Retrieve the body mass to calculate the needed force (F = m * a)
	float mass = 10;
	vec3 forceToApply = neededAcceleration * mass;

	// Only apply horizontal forces to avoid interfering with the vertical (gravity, jump, etc.)
	vec3 horizontalForce(forceToApply.x, 0.0f, forceToApply.z);


	// Apply the calculated force to the body
	LeadBody->AddForce(ToPhysics(horizontalForce));

	Physics::Activate(LeadBody);

	mesh->Rotation = vec3(0,MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);

}

void TestNpc::Serialize(json& target)
{

	Entity::Serialize(target);

	animationStateSaveData = mesh->GetAnimationState();

	Rotation = mesh->Rotation;

	SERIALIZE_FIELD(target, Rotation);
	SERIALIZE_FIELD(target, desiredDirection);
	SERIALIZE_FIELD(target, movingDirection);
	SERIALIZE_FIELD(target, speed);
	SERIALIZE_FIELD(target, dead);
	SERIALIZE_FIELD(target, animationStateSaveData);
	SERIALIZE_FIELD(target, attacking);
	SERIALIZE_FIELD(target, stuned);
	SERIALIZE_FIELD(target, attackingDamage);
	SERIALIZE_FIELD(target, fleeing);
}

void TestNpc::Deserialize(json& source)
{

	Entity::Deserialize(source);

	DESERIALIZE_FIELD(source, Rotation);
	DESERIALIZE_FIELD(source, desiredDirection);
	DESERIALIZE_FIELD(source, movingDirection);
	DESERIALIZE_FIELD(source, speed);
	DESERIALIZE_FIELD(source, dead);
	DESERIALIZE_FIELD(source, animationStateSaveData);
	DESERIALIZE_FIELD(source, attacking);
	DESERIALIZE_FIELD(source, stuned);
	DESERIALIZE_FIELD(source, attackingDamage);
	DESERIALIZE_FIELD(source, fleeing);


	Physics::SetBodyPosition(LeadBody, Position);


	if (dead)
	{
		Physics::DestroyBody(LeadBody);
		LeadBody = nullptr;

		DeathSoundPlayer->Destroy();
		HurtSoundPlayer->Destroy();
		StunSoundPlayer->Destroy();
		AttackSoundPlayer->Destroy();
		DeathSoundPlayer = nullptr;
		HurtSoundPlayer = nullptr;
		StunSoundPlayer = nullptr;
		AttackSoundPlayer = nullptr;

	}


	mesh->Rotation = Rotation;

	mesh->SetAnimationState(animationStateSaveData);
	mesh->Update(0);
	mesh->PullRootMotion();

}

void TestNpc::LoadAssets()
{

	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

	mesh->LoadFromFile("GameData/models/enemies/dog/dog.glb");
	mesh->CreateHitboxes(this);
	mesh->PlayAnimation("run",true);
	mesh->SetLooped(true);
	mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");

	SET_SOUND_SAFE(DeathSoundPlayer, FmodEventInstance::Create("event:/NPC/Dog/DogDeath"));
	SET_SOUND_SAFE(HurtSoundPlayer, SoundManager::GetSoundFromPath("GameData/sounds/dog/dog_hit.wav"));
	SET_SOUND_SAFE(StunSoundPlayer, FmodEventInstance::Create("event:/NPC/Dog/DogStun"));
	SET_SOUND_SAFE(AttackSoundPlayer, FmodEventInstance::Create("event:/NPC/Dog/DogAttackStart"));
	SET_SOUND_SAFE(AttackHitSoundPlayer, FmodEventInstance::Create("event:/NPC/Dog/DogAttack"));

}


