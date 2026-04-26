#include "WeaponFirearm.h"
#include <algorithm>
#include "Animators/WeaponAnimator.h"
#include <RandomHelper.h>
#include "Projectiles/Bullet.h"
#include <SoundSystem/FmodEventInstance.h>
#include "WeaponFireFlash.h"
#include <AiPerception/AiPerceptionSystem.h>
#include <AiPerception/Observer.h>
#include "../../Npc/NpcBase.h"

WeaponFirearm::WeaponFirearm(const FirearmParams& initialParams)
	: params(initialParams)
{
	LateUpdateWhenPaused = params.lateUpdateWhenPaused;
}

void WeaponFirearm::Start() {
	fireSoundPlayer = SoundPlayer::Create(params.fireSoundEvent);
	fireSoundPlayer->Volume = params.fireVolume;
	fireSoundPlayer->Is2D = params.fireSoundIs2D;

	attackDelay.AddDelay(params.switchDelayTime - 0.1f);
	SwitchDelay.AddDelay(params.switchDelayTime);

	AmmoType = params.ammoType;

	Update();
	AsyncUpdate();
	LateUpdate();
}

void WeaponFirearm::LoadAssets()
{
	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Weapons.bank");
	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/SFX.bank");

	if (thirdPersonAnimator == nullptr)
		thirdPersonAnimator = make_unique<WeaponAnimator>();
	thirdPersonAnimator->LoadAssetsIfNeeded();

	PreloadEntityType(params.bulletClass);

	// RIGHT HAND
	viewmodel = new SkeletalMesh(owner);
	arms = new SkeletalMesh(owner);
	viewmodel->LoadFromFile(params.modelPath);
	if (!params.texturesLocation.empty())
		viewmodel->TexturesLocation = params.texturesLocation;
	viewmodel->PlayAnimation(params.drawAnimation);
	viewmodel->PreloadAssets();
	viewmodel->Transparent = true;
	viewmodel->IsViewmodel = true;
	Drawables.push_back(viewmodel);

	arms->LoadFromFile(ArmsModelPath);
	arms->IsViewmodel = true;
	Drawables.push_back(arms);

	// LEFT HAND (always loaded)
	viewmodelLeft = new SkeletalMesh(this);
	armsLeft = new SkeletalMesh(this);

	if (params.modelPath.empty() == false)
	viewmodelLeft->LoadFromFile(params.modelPath);

	if (!params.texturesLocation.empty())
		viewmodelLeft->TexturesLocation = params.texturesLocation;
	viewmodelLeft->PlayAnimation(params.drawAnimation);
	viewmodelLeft->PreloadAssets();
	viewmodelLeft->Transparent = true;
	viewmodelLeft->IsViewmodel = true;
	Drawables.push_back(viewmodelLeft);

	armsLeft->LoadFromFile(ArmsModelPath);
	armsLeft->IsViewmodel = true;
	Drawables.push_back(armsLeft);

	viewmodelLeft->Scale = vec3(-1, 1, 1);
	armsLeft->Scale = vec3(-1, 1, 1);
	viewmodelLeft->TwoSided = true;
	armsLeft->TwoSided = true;

	// Third person model
	Drawables.push_back(thirdPersonModel = new SkeletalMesh(owner));

	if(params.modelPathTp.empty()== false)
		thirdPersonModel->LoadFromFile(params.modelPathTp);

	thirdPersonModel->TexturesLocation = params.texturesLocationTp;

	PreloadEntityType("bullet");

	// Initialize left models as hidden if akimbo off
	viewmodelLeft->Visible = false;
	armsLeft->Visible = false;
	akimboPrev = false;

	viewmodel->GravityAlignedRotation = true;
	arms->GravityAlignedRotation = true;
	viewmodelLeft->GravityAlignedRotation = true;
	armsLeft->GravityAlignedRotation = true;
	thirdPersonModel->GravityAlignedRotation = true;

	viewmodel->MeshCustomShaderParams["rim_pow"] = vec4(6);
	viewmodel->MeshCustomShaderParams["specular_pow"] = vec4(8);
	viewmodel->MeshCustomShaderParams["specular_scale"] = vec4(0.2);

}

void WeaponFirearm::SetAkimbo(bool enabled)
{
	akimbo = enabled;

	if (akimbo && !akimboPrev)
	{
		viewmodelLeft->Visible = true;
		armsLeft->Visible = true;
		viewmodelLeft->PlayAnimation(params.drawAnimation, false, 0);
	}
	if (!akimbo && akimboPrev)
	{
		viewmodelLeft->Visible = false;
		armsLeft->Visible = false;
	}

	akimboPrev = akimbo;
}

void WeaponFirearm::StopTrail(ParticleSystem* trail)
{

	if (trail == nullptr) return;

	trail->StopAll();
	trail->DestroyWithDelay(3);
}

void WeaponFirearm::Update()
{
	Weapon::Update();

	if (akimbo)
	{
		akimboDistanceProgress += Time::DeltaTimeF * 4.0f;
	}
	else
	{
		akimboDistanceProgress -= Time::DeltaTimeF * 4.0f;
	}

	akimboDistanceProgress = std::clamp(akimboDistanceProgress, 0.0f, 1.0f);

	// Detect akimbo change
	if (akimbo != akimboPrev)
		SetAkimbo(akimbo);

	weaponAim -= Time::DeltaTimeF * 1.5f;
	if (!CanAttack() && weaponAim > 1)
		weaponAim = 1.0f;

	oldWeaponAim = weaponAim;

	if (params.hasRecoilModelOffset) {
		if (attackDelay.Wait())
			recoilModelOffset = MathHelper::Interp(recoilModelOffset, params.recoilModelTarget, Time::DeltaTimeF, params.recoilModelInterpIn);
		else
			recoilModelOffset = MathHelper::Interp(recoilModelOffset, 0.0f, Time::DeltaTimeF, params.recoilModelInterpOut);
	}

	if (params.hasActiveSpread) {
		if (!attackDelay.Wait())
			activeSpread -= Time::DeltaTimeF * params.spreadDecreaseSpeed;
		activeSpread = std::clamp(activeSpread, 0.0f, params.maxActiveSpread);
		Spread = params.baseSpread + activeSpread + length(Player::Instance->controller.GetVelocity()) / params.velocitySpreadDivisor;
	}
	else
		Spread = params.baseSpread;

	bool firstPerson = owner != nullptr && owner->InThirdPerson() == false;
	if (!firstPerson)
		Spread *= 0.5f;

	if (Input::GetAction("attack")->Holding() && CanAttack() && !attackDelay.Wait())
		PerformAttack();
}

void WeaponFirearm::PerformAttack()
{

	if (owner->GetAmmo(params.ammoType) <= 0)
	{
		return;
	}

	if (params.hasActiveSpread)
		activeSpread += params.spreadIncreasePerShot;

	if (params.notifyNpcs)
		NotifyNpcs();

	if (params.activateViolenceCrime)
		Player::Instance->violanceCrimeActiveDelay.AddDelay(params.violenceCrimeDelay);

	if (params.useOneshotSound)
		SoundPlayer::PlayOneshot(params.fireSoundEvent, params.pitchModifier, params.fireVolume);
	else
	{
		fireSoundPlayer->Pitch = params.pitchModifier;
		fireSoundPlayer->Play();
	}

	bool firstperson = owner != nullptr && owner->ThirdPersonView == false;

	float shakeMultiplier = 1.0f;

	if (firstperson)
	{

	}
	else
	{
		shakeMultiplier = 0.3f;
	}

	if (params.hasRandomRecoilStrength) {
		float horizontalRecoilStrength = RandomHelper::RandomFloat() * 2 - 1;
		float verticalRecoilStrength = RandomHelper::RandomFloat() * 0.5f + 0.5f;
		CameraShake modifiedShake = params.recoilShake;
		modifiedShake.rotationAmplitude *= vec3(verticalRecoilStrength, horizontalRecoilStrength, 1);
		modifiedShake.rotationAmplitude *= shakeMultiplier;
		Camera::AddCameraShake(modifiedShake);
	}
	else {

		auto shake = params.recoilShake;

		shake.rotationAmplitude *= shakeMultiplier;

		Camera::AddCameraShake(shake);
	}

	SwitchDelay.AddDelay(params.switchDelayOnAttack * (akimbo ? 0.5f : 1));
	attackDelay.AddDelay(params.attackDelayTime * (akimbo ? 0.5f : 1.0f));


	bool fireLeft = akimbo && alternateFire && fireLeftNext;

	if (akimbo)
	{
		if (fireLeft)
			viewmodelLeft->PlayAnimation(params.fireAnimation, false, params.fireAnimInterpInTime);
		else
			viewmodel->PlayAnimation(params.fireAnimation, false, params.fireAnimInterpInTime);
	}
	else
		viewmodel->PlayAnimation(params.fireAnimation, false, params.fireAnimInterpInTime);

	fireLeftNext = !fireLeftNext;

	// muzzle selection
	mat4 boneMat = (akimbo && fireLeft ? viewmodelLeft : viewmodel)->GetBoneMatrixWorld(params.boneMuzzle);
	vec3 startLoc = MathHelper::DecomposeMatrix(boneMat).Position;
	startLoc = mix(startLoc, Camera::position, params.muzzleMix) - MathHelper::GetForwardVector(Camera::finalizedRotation) * params.muzzleForwardOffset;

	WeaponFireFlash::CreateAt(startLoc);

	int c = 0;

	// spread shooting
	if (params.spreadType == "grid") 
	{
		for (float y = -params.gridSpreadSize; y <= params.gridSpreadSize; y += params.gridStep) {
			for (float x = -params.gridSpreadSize; x <= params.gridSpreadSize; x += params.gridStep) {
				if (length(vec2(x, y)) > params.gridMaxLength) continue;
				c++;
				FireSingleBullet(startLoc, vec4(x, y, 0, 1));
			}
		}
	}
	else {
		for (int i = 0; i < params.bulletsPerShot; ++i)
			FireSingleBullet(startLoc, vec4(0));
	}

	owner->ConsumeAmmo(AmmoType, 1);

	if (fireLeft)
	{

		if (smokeTrailL == nullptr || smokeTrailL->emitters.empty() || smokeTrailL->emitters[0]->emitterTime > 1)
		{
			smokeTrailL = static_cast<ParticleSystem*>(Spawn("weapon_smoke"));
			smokeTrailL->Start();

		}

		stopEmittingSmokeDelayL.AddDelay(1);
		smokeTrailL->emitters[0]->emitterTime = 0;

	}
	else
	{

		if (smokeTrail == nullptr || smokeTrail->emitters.empty() || smokeTrail->emitters[0]->emitterTime > 1)
		{
			smokeTrail = static_cast<ParticleSystem*>(Spawn("weapon_smoke"));
			smokeTrail->Start();

		}

		stopEmittingSmokeDelay.AddDelay(1);
		smokeTrail->emitters[0]->emitterTime = 0;

	}


}

void WeaponFirearm::FireSingleBullet(const vec3& startLoc, const vec4& gridOffset)
{
	Bullet* bullet = static_cast<Bullet*>(LevelObjectFactory::instance().create(params.bulletClass));
	Level::Current->AddEntity(bullet);

	bullet->debuffOnHit = params.debuffOnHit;
	bullet->debuffStacks = params.debuffStacksOnHit;

	vec3 offset;
	if (params.spreadType == "grid")
		offset = MathHelper::GetRotationMatrix(Camera::finalizedRotation) * gridOffset;
	else
		offset = RandomHelper::RandomPosition(1) * Spread;

	vec3 endLoc;
	bool firstperson = owner != nullptr && owner->InThirdPerson() == false;

	if (firstperson)
		endLoc = Position + MathHelper::GetForwardVector(Camera::finalizedRotation) * params.range + offset;
	else {
		endLoc = Camera::position + MathHelper::GetForwardVector(Camera::finalizedRotation) * (params.range + 3);
		auto hit = Physics::LineTrace(Camera::finalizedPosition, endLoc, BodyType::GroupHitTest, {}, { owner });
		if (hit.hasHit) endLoc = hit.shapePosition;
		endLoc += offset * hit.fraction;
	}

	bullet->damageCauser = owner;
	bullet->Speed = params.bulletSpeed;
	bullet->Position = startLoc + offset * 0.002f;
	bullet->Rotation = MathHelper::FindLookAtRotation(startLoc, endLoc);
	bullet->Start();
	bullet->LoadAssetsIfNeeded();
	bullet->Damage = params.bulletDamage;
}

void WeaponFirearm::NotifyNpcs() 
{

	AiPerceptionSystem::EmitSoundAt(Position, params.npcNotifyRadius, (int)InvestigationReason::WeaponFire, Player::Instance->Id);

	auto observers = AiPerceptionSystem::GetObserversInRadius(Position, params.npcNotifyRadius);
	for (auto observer : observers) {
		auto ownerNpc = dynamic_cast<NpcBase*>(Level::Current->FindEntityWithId(observer->owner));
		if (ownerNpc)
		{
			ownerNpc->TryStartInvestigation(InvestigationReason::WeaponFire, Position, Player::Instance->Id);
		}
	}
}

void WeaponFirearm::AsyncUpdate()
{

	float hide = HideWeapon;

	if (akimbo)
	{
		hide = 1;
	}

	// RIGHT GUN
	viewmodel->Update();
	auto pose = viewmodel->GetAnimationPose();
	auto leftHandPose = pose.GetBoneTransform("clavicle_l");
	leftHandPose.Rotation += vec3(50, 0, 0) * hide;
	pose.SetBoneTransformEuler("clavicle_l", leftHandPose);
	arms->PasteAnimationPose(pose);

	// LEFT GUN
	if (akimbo)
	{
		viewmodelLeft->Update();
		auto poseL = viewmodelLeft->GetAnimationPose();
		auto leftHandPose = poseL.GetBoneTransform("clavicle_l");
		leftHandPose.Rotation += vec3(50, 0, 0) * hide;
		poseL.SetBoneTransformEuler("clavicle_l", leftHandPose);
		armsLeft->PasteAnimationPose(poseL);
	}

	if (smokeTrail)
	{
		mat4 boneMat = (viewmodel)->GetBoneMatrixWorld("weapon_fire_point");
		vec3 startLoc = MathHelper::DecomposeMatrix(boneMat).Position;

		smokeTrail->Position = startLoc;
		
		if (stopEmittingSmokeDelay.Wait() == false)
		{
			StopTrail(smokeTrail);
		}

	}

	if (smokeTrailL)
	{
		mat4 boneMat = (viewmodelLeft)->GetBoneMatrixWorld("weapon_fire_point");
		vec3 startLoc = MathHelper::DecomposeMatrix(boneMat).Position;

		smokeTrailL->Position = startLoc;

		if (stopEmittingSmokeDelayL.Wait() == false)
		{
			StopTrail(smokeTrailL);
		}

	}


}

void WeaponFirearm::LateUpdate()
{

	vec3 offset = params.weaponOffset;

	offset.x += akimboDistanceProgress * -0.012f;

	viewmodel->Position = Position + (mat3)Camera::GetRotationMatrix() * offset;
	viewmodel->Rotation = Rotation;
	if (params.hasRecoilModelOffset) viewmodel->Rotation.x += recoilModelOffset;
	viewmodel->Visible = owner != nullptr && owner->InThirdPerson() == false;
	arms->Visible = viewmodel->Visible;
	arms->Position = viewmodel->Position;
	arms->Rotation = viewmodel->Rotation;

	thirdPersonModel->Visible = !viewmodel->Visible;
	thirdPersonModel->Position = owner->bodyMesh->Position;
	thirdPersonModel->Rotation = owner->bodyMesh->Rotation;

	if (akimbo)
	{

		viewmodelLeft->Visible = viewmodel->Visible;
		armsLeft->Visible = arms->Visible;
		viewmodelLeft->Position = Position + (mat3)Camera::GetRotationMatrix() * (offset * vec3(-1,1,1));
		viewmodelLeft->Rotation = Rotation;
		if (params.hasRecoilModelOffset) viewmodelLeft->Rotation.x += recoilModelOffset;
		armsLeft->Position = viewmodelLeft->Position;
		armsLeft->Rotation = viewmodelLeft->Rotation;
	}
}

WeaponSlotData WeaponFirearm::GetDefaultData() {
	WeaponSlotData data;
	data.className = "firearm";
	data.slot = 0;
	return data;
}

void WeaponFirearm::Destroy()
{

	Weapon::Destroy();
	StopTrail(smokeTrail);
	StopTrail(smokeTrailL);
}

AnimationPose WeaponFirearm::ApplyWeaponAnimation(AnimationPose thirdPersonPose)
{
	thirdPersonAnimator->weaponAim = std::clamp(weaponAim,0.0f, 1.0f);
	thirdPersonAnimator->inPose = thirdPersonPose;
	thirdPersonAnimator->Update();
	lastAppliedPose = thirdPersonAnimator->GetResultPose();
	thirdPersonModel->PasteAnimationPose(lastAppliedPose);
	return lastAppliedPose;
}
