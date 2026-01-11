#include "NpcHumanGun.h"
#include "../../Player/Player.hpp"
#include "../../Player/Weapons/Projectiles/Bullet.h"

#include <Particle/GlobalParticleSystem.hpp>
#include <SoundSystem/FmodEventInstance.h>
#include <Navigation/Navigation.hpp>
#include <RandomHelper.h>
#include "NpcHelper.h"


REGISTER_ENTITY(NpcHumanGun, "npc_human_gun")

void NpcHumanGun::UpdateFleeTarget()
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

void NpcHumanGun::PlaySoundEffect(std::string eventName)
{

	if (soundPlayer == nullptr) return;

	soundPlayer->SetSound(FmodEventInstance::Create(eventName));

	soundPlayer->Play();

}

void NpcHumanGun::ProcessAnimationEvent(AnimationEvent& event)
{

	if (event.eventName == "stun_end")
	{
		stuned = false;
		mesh->PlayAnimation("run", true, 0.5f);
	}

}

void NpcHumanGun::Start()
{

	mesh->Position = Position - vec3(0, 1, 0);
	mesh->Rotation = Rotation;

	//Drawables.push_back(mesh);

	LeadBody = Physics::CreateCharacterBody(this, Position, 0.5, 2, 50);


	Physics::SetGravityFactor(LeadBody, 4);

	desiredDirection = MathHelper::XZ(MathHelper::GetForwardVector(Rotation));
	movingDirection = desiredDirection;

	pathFollow.CalculatePathOnThread();

	soundPlayer = SoundPlayer::Create();


	SetupSoundPlayer(soundPlayer);

}

void NpcHumanGun::Stun()
{
	stuned = true;
	mesh->PlayAnimation("stun");
	mesh->PullRootMotion();

	PlaySoundEffect("event:/NPC/Enemy1/Enemy1Stun");

}

void NpcHumanGun::Attack()
{
	if (cantAttackDelay.Wait())return;
	if (inAttackDelay.Wait())return;


	vec3 bonePos = mesh->GetBoneMatrixWorld("weapon_muzzle")[3];

	Entity* targetRef = Player::Instance;

	vec3 predictedTargetPosition = targetRef->Position;

	if (AttackDirectionCheck(bonePos, predictedTargetPosition, targetRef) == false)
	{
		inAttackDelay.AddDelay(0.5f);
	}

	const float bulletSpeed = 50;

	if (targetRef)
	{

		Player* playerRef = dynamic_cast<Player*>(targetRef);

		if (playerRef)
		{
			predictedTargetPosition += playerRef->controller.GetVelocity() * distance(Position, targetRef->Position) / bulletSpeed;
		}
		else
		{
			if (targetRef->LeadBody)
			{
				predictedTargetPosition += FromPhysics(targetRef->LeadBody->GetLinearVelocity()) * distance(Position, targetRef->Position) / bulletSpeed;
			}
		}


	}

	predictedTargetPosition += RandomHelper::RandomPosition(distance(targetRef->Position, Position)/20.0f) * (accuracyModifier + 1.0f);

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
	mesh->PlayAnimation("fire");
	mesh->PullRootMotion();

}

void NpcHumanGun::Death()
{

	if (dead) return;

	//mesh->ClearHitboxes();
	mesh->StartRagdoll();
	mesh->SetAnimationPaused(true);
	Physics::SetLinearVelocity(LeadBody, vec3(0));

	PlaySoundEffect("event:/NPC/Enemy1/Enemy1Death");

	GetDebuffsList().clear();
	UpdateStatusWidgets();

	//Physics::SetBodyType(LeadBody, BodyType::None);
	//Physics::SetCollisionMask(LeadBody, BodyType::World);

	Physics::DestroyBody(LeadBody);
	LeadBody = nullptr;

	CallActionOnEntityWithId(OwnerId, "despawned");

	dead = true;

	if (soundPlayer)
	{
		soundPlayer->DestroyWithDelay(3);
		soundPlayer = nullptr;
	}

	//Tags.clear();

}

void NpcHumanGun::OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon)
{
	Damage *= mesh->GetHitboxDamageMultiplier(bone);
	Entity::OnPointDamage(Damage, Point, Direction, bone, DamageCauser, Weapon);

	if (mesh->GetHitboxDamageMultiplier(bone) > 1.1f && dead)
	{
		//Time::AddTimeScaleEffect(0.3f, 0.15f, true, "hit_slow");
	}

	GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point, MathHelper::FindLookAtRotation(vec3(0), Direction), vec3(Damage / 10.0f));

	SoundPlayer::PlayOneshot("event:/NPC/General/FleshHit", 1, Damage / 20.0f, false, Point);

}

void NpcHumanGun::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{

	Damage = ModifyIncomingDamage(Damage);

	accuracyModifier += Damage / 30.0f;

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
			PlaySoundEffect("event:/NPC/Enemy1/Enemy1Damage");
		}
	}

	if (Health < 30)
	{
		//fleeing = true;
		//UpdateFleeTarget();
	}

}

bool NpcHumanGun::LineOfSightCheck(Entity* targetEntity)
{

	auto hit = Physics::SphereTrace(Position + vec3(0,0.4f,0), targetEntity->Position + vec3(0,0.3f,0),0.4,
		BodyType::World,
		{  }, {this, targetEntity});

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

	accuracyModifier -= Time::DeltaTimeF / 3.0f;
	accuracyModifier = glm::clamp(accuracyModifier, 0.0f, 5.0f);

	UpdateStatusWidgets();

	//mesh->UpdatePose = mesh->WasRended;

	UpdateDebuffs(Time::DeltaTimeF);

	mesh->Update(ModifyAnimationSpeed(1.0f));


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


	if (dead || stuned) return;

	soundPlayer->Position = Position;


	soundPlayer->Velocity = FromPhysics(LeadBody->GetLinearVelocity());

	auto animName = mesh->GetAnimationName();

	if (animName == "run" || animName == "stun")
	{
		cantAttackDelay.AddDelay(0.7f);
	}

	Entity* target = Player::Instance;

	desiredTargetLocation = target->Position;


	vec3 lookAtDir = MathHelper::FastNormalize(target->Position - Position);

	bool hasLineOfSight = LineOfSightCheck(target);

	float attackDistance = 17;

	if (animName == "run")
	{
		attackDistance = 13;
	}

	if (distance2(target->Position, Position) > attackDistance * attackDistance)
	{
		hasLineOfSight = false;
	}

	if (hasLineOfSight)
	{

		if (animName == "run")
		{
			mesh->PlayAnimation("aim", true, 0.3f);
			
		}
		else
		{
			desiredDirection = lookAtDir;
		}

		if (cantAttackDelay.Wait() == false)
		{
			Attack();
		}

	}
	else
	{
		if (animName != "run" && stuned == false)
		{
			mesh->PlayAnimation("run", true, 0.4f);
		}
		
		cantAttackDelay.AddDelay(0.5f);

	}


	if (target)
	{
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

	}
	if (pathFollow.FoundTarget && animName == "run")
	{
		desiredDirection = normalize(MathHelper::XZ(pathFollow.CalculatedTargetLocation - Position));
	}


	speed += Time::DeltaTimeF * 6.5;

	speed = glm::clamp(speed, 0.0f, ModifyMovementSpeed(maxSpeed));

	if (hasLineOfSight || inAttackDelay.Wait())
	{
		speed = 0;
	}

	movingDirection = mix(movingDirection, desiredDirection, Time::DeltaTime * 10);

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
	float mass = 40;
	vec3 forceToApply = neededAcceleration * mass;

	// Only apply horizontal forces to avoid interfering with the vertical (gravity, jump, etc.)
	vec3 horizontalForce(forceToApply.x, 0.0f, forceToApply.z);


	// Apply the calculated force to the body
	LeadBody->AddForce(ToPhysics(horizontalForce));

	Physics::Activate(LeadBody);

	mesh->Rotation = vec3(0, MathHelper::FindLookAtRotation(vec3(), movingDirection).y, 0);

}

void NpcHumanGun::Serialize(json& target)
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
	SERIALIZE_FIELD(target, stuned);
	SERIALIZE_FIELD(target, fleeing);

	SERIALIZE_FIELD(target, inAttackDelay);
	SERIALIZE_FIELD(target, cantAttackDelay);
	SERIALIZE_FIELD(target, desiredTargetLocation);


	auto debuffsJson = SerializeDebuffs();
	SERIALIZE_FIELD(target, debuffsJson);

}

void NpcHumanGun::Deserialize(json& source)
{

	Entity::Deserialize(source);

	DESERIALIZE_FIELD(source, Rotation);
	DESERIALIZE_FIELD(source, desiredDirection);
	DESERIALIZE_FIELD(source, movingDirection);
	DESERIALIZE_FIELD(source, speed);
	DESERIALIZE_FIELD(source, dead);
	DESERIALIZE_FIELD(source, animationStateSaveData);
	DESERIALIZE_FIELD(source, stuned);
	DESERIALIZE_FIELD(source, fleeing);

	DESERIALIZE_FIELD(source, inAttackDelay);
	DESERIALIZE_FIELD(source, cantAttackDelay);
	DESERIALIZE_FIELD(source, desiredTargetLocation);

	json debuffsJson;
	DESERIALIZE_FIELD(source, debuffsJson);
	DeserializeDebuffs(debuffsJson);

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

void NpcHumanGun::UpdateStatusWidgets()
{

	statusWidget->Position = Position + vec3(0, 1.0f, 0);

	statusWidget->TwoSided = true;
	statusWidget->Update();

	statusWidget->Visible = !dead;

	//if (GetDebuffsList().empty())
	//{
	//	statusWidget->Visible = false;
	//}
	//else
	//{
	//	statusWidget->Visible = true;
	//	statusWidget->Update();
	//}

}

void NpcHumanGun::UpdateDebugUI()
{

	ImGui::Begin(("NpcHumanAxe Debug: " + Id).c_str());

	ImGui::Text(("Health: " + std::to_string(Health)).c_str());

	ImGui::Text(GetDebuffsDebugInfo().c_str());

	ImGui::End();

}

void NpcHumanGun::LoadAssets()
{

	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

	mesh->TexturesLocation = "GameData/models/enemies/humanGun/humanGun.glb/";
	mesh->LoadFromFile("GameData/models/enemies/humanGun/humanGun.glb");
	mesh->PreloadAssets();
	mesh->CreateHitboxes(this);
	mesh->PlayAnimation("run", true);
	mesh->SetLooped(true);
	mesh->Scale = vec3(1.15f);
	//mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");

}
