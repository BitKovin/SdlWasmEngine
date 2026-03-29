#include "TestNpc.hpp"
#include "../../Player/Player.hpp"

#include <Particle/GlobalParticleSystem.hpp>
#include <SoundSystem/FmodEventInstance.h>
#include <Navigation/Navigation.hpp>
#include "NpcHelper.h"

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

	soundPlayer = SoundPlayer::Create();


	SetupSoundPlayer(soundPlayer);
}

void TestNpc::Stun()
{

	if (dead) return;

	stuned = true;
	mesh->PlayAnimation("stun");
	mesh->PullRootMotion();
	attacking = false;
	attackingDamage = false;

	
	PlaySoundEffect("event:/NPC/Dog/DogStun");

}

void TestNpc::Attack()
{

	PlaySoundEffect("event:/NPC/Dog/DogAttackStart");

	inAttackDelay.AddDelay(4.5f);
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

	PlaySoundEffect("event:/NPC/Dog/DogDeath");

	//Physics::SetBodyType(LeadBody, BodyType::None);
	//Physics::SetCollisionMask(LeadBody, BodyType::World);

	Physics::DestroyBody(LeadBody);
	LeadBody = nullptr;

	CallActionOnEntityWithId(OwnerId, "despawned");

	dead = true;


	//Tags.clear();

}

void TestNpc::OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon)
{

	if (dead) return;

	Damage *= mesh->GetHitboxDamageMultiplier(bone);
	Entity::OnPointDamage(Damage, Point, Direction, bone, DamageCauser, Weapon);

	if (mesh->GetHitboxDamageMultiplier(bone) > 1.1f && dead)
	{
		//Time::AddTimeScaleEffect(0.3f, 0.15f, true, "hit_slow");
	}

	GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point, MathHelper::FindLookAtRotation(vec3(0), Direction), vec3(Damage/10.0f));

	SoundPlayer::PlayOneshot("event:/NPC/General/FleshHit", 1, Damage / 20.0f, false, Point);

}

void TestNpc::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{

	Damage = ModifyIncomingDamage(Damage);

	Health -= Damage;

	if (Health <= 0)
	{
		Death();
	}

	if (DamageCauser != nullptr)
	{
		if (LeadBody)
		{
			LeadBody->SetLinearVelocity(LeadBody->GetLinearVelocity() / 2.0f);
			speed /= 2.0f;
			//HurtSoundPlayer->Play();
		}
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
				Stun();
				OnPointDamage(10, hit.shapePosition, MathHelper::FastNormalize(Position - hit.shapePosition), "", this, this);
				AddDebuffStacks("PoiseBreakDebuff", 100);
				attackingDamage = false;
				return;
			}
			else
			{
				hit.entity->OnPointDamage(ModifyOutgoingDamage(10), hit.shapePosition, MathHelper::FastNormalize(hit.shapePosition - Position), "", this, this);
				attackingDamage = false;


				PlaySoundEffect("event:/NPC/Dog/DogAttack");
			}

		}
	}

}

void TestNpc::AsyncUpdate()
{

	if (dead)
	{

	}

	UpdateStatusWidgets();

	UpdateDebuffs(Time::DeltaTimeF);

	//mesh->UpdatePose = mesh->WasRended;

	mesh->Update(ModifyAnimationSpeed(1));
	

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
		Position += MathHelper::ClampLength(rootMotion.Position, 0.0f, 0.2f);
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

	soundPlayer->Position = Position;


	soundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());


	Entity* target = Player::Instance;

	if (attacking)
	{


		return;
	}

	vec3 lookAtDir = MathHelper::FastNormalize(target->Position - Position);

	if (distance(target->Position, Position) < 5 
		&& dot(MathHelper::GetForwardVector(mesh->Rotation), lookAtDir) > 0.975)
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

	speed = glm::clamp(speed, 0.0f, ModifyMovementSpeed(maxSpeed));

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

	if (EngineMain::MainInstance->LoadingFrames > 0)
	{
		horizontalForce = vec3(0); //weird delta time during loading
	}
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

		soundPlayer->Destroy();
		soundPlayer = nullptr;


	}


	mesh->Rotation = Rotation;

	mesh->SetAnimationState(animationStateSaveData);
	mesh->Update(0);
	mesh->PullRootMotion();

}

void TestNpc::UpdateStatusWidgets()
{

	statusWidget->Position = Position + vec3(0, 1.0f, 0);

	statusWidget->TwoSided = true;
	statusWidget->Update();

	statusWidget->Visible = !dead;

}

void TestNpc::LoadAssets()
{

	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

	mesh->LoadFromFile("GameData/models/enemies/dog/dog.glb");
	mesh->CreateHitboxes(this);
	mesh->PlayAnimation("run",true);
	mesh->SetLooped(true);
	mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");
	mesh->CustomId = 1;

}

void TestNpc::PlaySoundEffect(std::string eventName)
{

	if (soundPlayer == nullptr) return;

	soundPlayer->SetSound(FmodEventInstance::Create(eventName));

	soundPlayer->Play();

}


