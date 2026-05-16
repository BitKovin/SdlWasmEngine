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
	mesh->MeshCustomShaderParams["rim_color"] = vec4(1.5f, 1.2f, 0.0f, 1.0f);
	mesh->MeshCustomShaderParams["rim_pow"] = vec4(2.0f);

	//Drawables.push_back(mesh);

    controller.Init(this, Position, 0.5f, 2.0f);

	Logger::Log("started npc\n");

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

	mesh->StartRagdoll();
	mesh->SetAnimationPaused(true);
	controller.SetVelocity(vec3(0));

	PlaySoundEffect("event:/NPC/Dog/DogDeath");

	controller.Destroy();

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
	GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point, MathHelper::FindLookAtRotation(Direction, vec3(0)), vec3(Damage / 10.0f));

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
		controller.SetVelocity(controller.GetVelocity() / 2.0f);
		speed /= 2.0f;
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

	auto hit = Physics::SphereTrace(Position, MathHelper::GetForwardVector(mesh->Rotation)*0.75f + Position, 0.2f, BodyType::World | BodyType::CharacterCapsule, {}, {this});

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

	if (!dead)
	{
		controller.Update(Time::DeltaTimeF);
		Position = controller.GetPosition();
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

	if (length(rootMotion.Position) > 0.1)
	{

		auto hit = Physics::SphereTrace(Position, Position + rootMotion.Position, 0.2, BodyType::GroupCollisionTest, {}, {this});

		if(hit.hasHit)
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


	mesh->UpdateHitboxes();

	if (dead || stuned) return;

	UpdateAttackDamage();

	if (dead || stuned) return;

	soundPlayer->Position = Position;


	soundPlayer->Velocity = controller.GetVelocity();


	Entity* target = Player::Instance;

	if (attacking)
	{


		return;
	}

	vec3 calculatedTargetPos = target->Position;

	if (target->Position.y < Position.y - 0.1f && target->Position.y > Position.y - 1.5f)
	{
		calculatedTargetPos.y = Position.y;
	}

	vec3 lookAtDir = MathHelper::FastNormalize(calculatedTargetPos - Position);

	if (distance(calculatedTargetPos, Position) < 5 
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

	if (EngineMain::MainInstance->LoadingFrames == 0)
	{
		vec3 vel = controller.GetVelocity();
		controller.SetVelocity(vec3(movingDirection.x * speed, vel.y, movingDirection.z * speed));
	}

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


	controller.SetPosition(Position);

	if (dead)
	{
		controller.Destroy();

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

	statusWidget->Visible = !dead && widgetVisible;

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


