#include "WeaponBase.h"
#include "../Player.hpp"

#include <Animation.h>

#include "Projectiles/CaneProjectile.h"
#include <SoundSystem/FmodEventInstance.h>
#include "../../Enemy/IEnemy.h"


class weapon_cane : public Weapon
{
public:

	SkeletalMesh* viewmodel;
	SkeletalMesh* arms;

	vec3 weaponOffset = vec3(0.0, 0.00, -0.0);

	Delay attackDelay;

	vec3 projectileOffset = vec3(0.03f, -0.15f, -0.3f);


	const Body* bodyToPush = nullptr;
	vec3 impulseToApply = vec3();
	Delay pushDelay = Delay(1000000000);

	Delay grabDelay;
	bool grabing = false;

	vec3 grabStartPos = vec3();

	bool thrown = false;


	bool pendingMeleeAttack = false;

	Delay parryDelay;

	weapon_cane()
	{

		LateUpdateWhenPaused = true;
	}

	SoundPlayer* fireSoundPlayer = nullptr;

	void Start()
	{

		fireSoundPlayer = SoundPlayer::Create("event:/Weapons/pistol/pistol_fire");
		fireSoundPlayer->Volume = 0.5f;
		fireSoundPlayer->Is2D = true;


		//attackDelay.AddDelay(0.3);
		SwitchDelay.AddDelay(0.35);

		pushDelay.AddDelay(100000000);

	}

	void SetViewmodelScaleFactor(float factor)
	{
		viewmodel->ViewmodelScaleFactor = factor;
		arms->ViewmodelScaleFactor = factor;
	}

	void LoadAssets()
	{

		SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Weapons.bank");
		SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

		viewmodel = new SkeletalMesh(this);
		arms = new SkeletalMesh(this);

		viewmodel->LoadFromFile("GameData/models/player/weapons/cane/cane.glb");
		//viewmodel->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/textures/cat.png");
		viewmodel->TexturesLocation = "GameData/models/player/weapons/cane/cane.glb/"; // to search in file:   
		viewmodel->PlayAnimation("idle",true);
		viewmodel->PreloadAssets();

		viewmodel->IsViewmodel = true;

		Drawables.push_back(viewmodel);

		arms->LoadFromFile(ArmsModelPath);
		arms->IsViewmodel = true;
		Drawables.push_back(arms);

		PreloadEntityType("caneProjectile");

	}

	void ReturnCane()
	{

		auto projectiles = Level::Current->FindAllEntitiesWithName("caneProjectile");

		for (auto p : projectiles)
		{
			p->Destroy();
		}

		viewmodel->PlayAnimation("idle", true, 0.3f);

		thrown = false;

		attackDelay.AddDelay(0.3);

	}

	void GrabCane()
	{

		attackDelay.AddDelay(1);

		MathHelper::Transform projectileTransform;

		auto projectiles = Level::Current->FindAllEntitiesWithName("caneProjectile");

		for (auto p : projectiles)
		{

			CaneProjectile* proj = (CaneProjectile*)p;

			if (proj != nullptr)
			{

				proj->DamageEntity();

				proj->Destroy();

				projectileTransform.Position = proj->Position;
				projectileTransform.Rotation = proj->Rotation;

				bodyToPush = proj->bodyToPush;
				impulseToApply = proj->impulseToApply;
				pushDelay.AddDelay(0.07f);

			}

		}

		vec3 safePosition = projectileTransform.Position + MathHelper::GetForwardVector(projectileTransform.Rotation) * - 0.3f;

		vec3 directionToPlayer = normalize(MathHelper::XZ((MathHelper::GetForwardVector(projectileTransform.Rotation) * -1.0f)));

		vec3 playerToCameraDif = Camera::position - Player::Instance->Position;



		auto hit = Physics::SphereTrace(safePosition, safePosition - vec3(0, 1, 0), 0.001f, BodyType::World | BodyType::MainBody);



		if (hit.hasHit)
		{
			safePosition = hit.shapePosition + hit.normal;
		}

		Player::Instance->MoveTo(safePosition + directionToPlayer - playerToCameraDif + vec3(0, 0.1f, 0));



		SetViewmodelScaleFactor(0.5);

		viewmodel->PlayAnimation("grab", false, 0.0f);
		Time::AddTimeScaleEffect(0.65, 0.2, true, "weapon", 0.15f, 0.2);


		thrown = false;

	}

	void StartGrab()
	{
		grabing = true;
		thrown = false;

		grabDelay.AddDelay(0.12f);

		attackDelay.AddDelay(1);

		viewmodel->PlayAnimation("take", false, 0);

		grabStartPos = Player::Instance->Position;


		SoundPlayer::PlayOneshot("event:/General/BassDrop", 3, 1, true);

	}

	void UpdateGrab()
	{
		if (!grabing) return;

		if (grabDelay.GetProgress() >= 1)
		{

			grabing = false;

			GrabCane();

			return;
		}

		viewmodel->SetAnimationTime(viewmodel->GetAnimationDuration()*grabDelay.GetProgress());

		auto projectiles = Level::Current->FindAllEntitiesWithName("caneProjectile");

		MathHelper::Transform projectileTransform;

		for (auto p : projectiles)
		{

			CaneProjectile* proj = (CaneProjectile*)p;

			if (proj != nullptr)
			{

				projectileTransform.Position = proj->Position;
				projectileTransform.Rotation = proj->Rotation;
				proj->movingTo = true;


			}

		}

		vec3 safePosition = projectileTransform.Position + MathHelper::GetForwardVector(projectileTransform.Rotation) * -0.3f;

		vec3 directionToPlayer = normalize(MathHelper::XZ((MathHelper::GetForwardVector(projectileTransform.Rotation) * -1.0f)));

		vec3 playerToCameraDif = Camera::position - Player::Instance->Position;



		auto hit = Physics::SphereTrace(safePosition, safePosition - vec3(0, 1, 0), 0.001f, BodyType::World | BodyType::MainBody);



		if (hit.hasHit)
		{
			safePosition = hit.shapePosition + hit.normal;
		}

		vec3 destinationPos = safePosition + directionToPlayer - playerToCameraDif + vec3(0, 0.1f, 0);

		printf("%f \n", grabDelay.GetProgress());

		Player::Instance->MoveTo(lerp(grabStartPos, destinationPos, grabDelay.GetProgress()));




	}

	void OnParried() 
	{

		Time::AddTimeScaleEffect(0.3, 0.1, true, "parry", 0.02f, 0.1f);

		//Time::AddTimeScaleEffect(0.2, 0.2, true, "parry2", 0.1f, 0.1f);

		SoundPlayer::PlayOneshot("event:/Weapons/cane/cane_parry", 1.0f, 1.0f, false);

		if (viewmodel->GetAnimationTime() < 0.18)
			viewmodel->SetAnimationTime(0.18f);

		pendingMeleeAttack = false;

	}

	void PerformParry()
	{

		SoundPlayer::PlayOneshot("event:/Weapons/knife/knife_attack", 2, 1, false);

		SetViewmodelScaleFactor(0.2f);

		parryDelay.AddDelay(0.3f);

		viewmodel->PlayAnimation("attack", false, 0.1f);

		attackDelay.AddDelay(0.75f);

		pendingMeleeAttack = true;

	}

	void PerformMeleeAttack()
	{

		pendingMeleeAttack = false;
		auto hit = Physics::SphereTrace(Camera::position, Camera::position + MathHelper::GetForwardVector(Camera::rotation) * 1.2f, 0.4f, BodyType::CharacterCapsule | BodyType::World, { Player::Instance->LeadBody }, { Player::Instance });
		if (hit.hasHit)
		{

			float damage = 20.0f;

			IEnemy* enemy = dynamic_cast<IEnemy*>(hit.entity);

			if (enemy->HasDebuff("DisbalanceDebuff"))
			{
				damage = 35;
			}


			if (hit.entity != nullptr)
			{
				hit.entity->OnPointDamage(damage, hit.position, MathHelper::GetForwardVector(Camera::rotation), hit.hitboxName, Player::Instance, this);
				Physics::AddImpulseAtLocation(hit.hitbody, MathHelper::GetForwardVector(Camera::rotation) * damage * 25.0f, hit.position);
			}

			if (enemy != nullptr)
			{
				enemy->AddDebuffStacks("PoiseBreakDebuff", 30);
			}

		}
	}

	void Update()
	{

		auto projectile = (CaneProjectile*) Level::Current->FindEntityWithName("caneProjectile");

		thrown = projectile != nullptr;

		Parrying = parryDelay.Wait();

		if (Parrying == false && pendingMeleeAttack)
		{
			PerformMeleeAttack();
		}

		if (Input::GetAction("attack2")->Pressed())
		{
			if (attackDelay.Wait() == false)
			{

				if (true)
				{

					PerformParry();

				}
				else
				{

					if (thrown)
					{

						if (projectile->inEnemy)
						{
							StartGrab();
						}
						else
						{
							ReturnCane();
						}


					}
					else
					{
						Attack();
					}

				}

			}
		}

		UpdateGrab();

		if (pushDelay.Wait() == false)
		{

			pushDelay.AddDelay(100000000);

			if (bodyToPush != nullptr)
			{
				
				auto hitMesh = Physics::GetBodyData(bodyToPush)->OwnerSkeletalMesh;

				if (hitMesh)
				{
					hitMesh->ApplyImpulseToAllHitboxes(impulseToApply*0.01f, true);
					Physics::AddImpulse(bodyToPush, impulseToApply * 0.25f);
				}
				else
				{
					Physics::AddImpulse(bodyToPush, impulseToApply);
				}


				bodyToPush = nullptr;

			}

		}

		if (attackDelay.Wait() == false)
		{

			projectile = (CaneProjectile*) Level::Current->FindEntityWithName("caneProjectile");

			if (projectile == nullptr && viewmodel->currentAnimationData->animationName == "throw")
			{

				ReturnCane();

			}
			else
			{
				if (projectile != nullptr && attackDelay.Wait() == false)
				{
					if (viewmodel->currentAnimationData->animationName != "throw")
					{
						viewmodel->PlayAnimation("throw", false, 0);
						viewmodel->SetAnimationTime(viewmodel->GetAnimationDuration() - 0.5f);
						viewmodel->Update();
						viewmodel->PullAnimationEvents();
					}

				}

			}

		}
		

		auto events = viewmodel->PullAnimationEvents();

		for (auto event : events)
		{
			if (event.eventName == "throw")
			{
				PerformAttack();
			}

			if (event.eventName == "restore_size")
			{
				SetViewmodelScaleFactor(2);
			}
		}

	}

	void Attack()
	{

		SwitchDelay.AddDelay(0.2f);

		SetViewmodelScaleFactor(2);

		viewmodel->PlayAnimation("throw", false, 0);
		Camera::AddCameraShake(CameraShake(
			0.13f,                            // interpIn
			0.0f,                            // duration
			vec3(0.0f, 0.0f, -0.1f),         // positionAmplitude
			vec3(0.0f, 0.0f, 3.4f),          // positionFrequency
			vec3(-4, 0.15f, 0.0f),        // rotationAmplitude
			vec3(-2.0f, 18.8f, 0.0f),        // rotationFrequency
			0.5f,                            // falloff
			CameraShake::ShakeType::SingleWave // shakeType
		));



		attackDelay.AddDelay(1.0f);

	}

	void UpdateDebugUI()
	{

		//ImGui::Begin("cane options");

		//ImGui::DragFloat3("projectile offset", &projectileOffset.x, 0.01f);

		//ImGui::End();

	}

	void PerformAttack()
	{

		vec3 startLoc = Camera::position +
			MathHelper::TransformVector(projectileOffset,
				Camera::GetRotationMatrix());


		auto projectiles = Level::Current->FindAllEntitiesWithName("caneProjectile");

		for (auto p : projectiles)
		{
			p->Destroy();
		}

		CaneProjectile* bullet = new CaneProjectile();
		bullet->owner = Player::Instance;
		Level::Current->AddEntity(bullet);

		vec4 offset = vec4(0);

		vec3 endLoc = Position + MathHelper::GetForwardVector(Camera::rotation) * 80.0f + vec3(offset);

		bullet->Speed = 60.f;
		bullet->MaxDistance = 120;
		bullet->Position = startLoc + vec3(offset) * 0.002f;
		bullet->Rotation = MathHelper::FindLookAtRotation(startLoc, endLoc);
		bullet->Start();
		bullet->LoadAssetsIfNeeded();
		bullet->Damage = 60;

		//fireSoundPlayer->Play();

	}

	void AsyncUpdate()
	{
		viewmodel->Update();

		auto pose = viewmodel->GetAnimationPose();

		arms->PasteAnimationPose(pose);
	}

	void LateUpdate()
	{
		viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * weaponOffset;
		viewmodel->Rotation = Rotation;

		arms->Position = viewmodel->Position;
		arms->Rotation = viewmodel->Rotation;

		viewmodel->Visible = !thrown;
	}


	WeaponSlotData GetDefaultData() override
	{

		WeaponSlotData data;

		data.className = "weapon_cane";

		return data;
	}


private:

};

REGISTER_ENTITY(weapon_cane, "weapon_cane")