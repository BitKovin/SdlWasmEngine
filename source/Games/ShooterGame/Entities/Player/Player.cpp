#include "Player.hpp"

#include <Particle/GlobalParticleSystem.hpp>

#include <EngineMain.h>

#include <AiPerception/AiPerceptionSystem.h>

#include "Weapons/WeaponFirearm.h"

#include "RestrictedArea.h"

REGISTER_ENTITY(Player, "player")

Player* Player::Instance = nullptr;

string serializedPlayer = "";

void Player::Start()
{

    Entity::Start();

    Instance = this;

    observationTarget = AiPerceptionSystem::CreateTarget(Position, Id, {"player"});

    controller.Init(this, Position, 0.4f);
    oldPos = controller.GetPosition();

    ParticleSystem::PreloadSystemAssets("decal_blood");
    ParticleSystem::PreloadSystemAssets("hit_flesh");


    soundPlayer = new SoundPlayer();
    Level::Current->AddEntity(soundPlayer);
    soundPlayer->Sound = SoundManager::GetSoundFromPath("GameData/sounds/mew.wav");

    Hud.Init(this);



    hitbox = Physics::CreateCharacterBody(this, Position, 0.3f, 1.2f, 0.1f, BodyType::HitBox, BodyType::None);
    hitbox->SetMotionType(JPH::EMotionType::Kinematic);
	Physics::ExcludedDrawBodies.insert(hitbox);

    PreloadEntityType("weapon_pistol");
    PreloadEntityType("weapon_shotgun");
    PreloadEntityType("weapon_tommy");
    PreloadEntityType("weapon_sniper");
    PreloadEntityType("weapon_swords");

    PreloadEntityType("weapon_lefthand_empty");

    PreloadEntityType("weapon_cane");

    AddWeaponByName("weapon_pistol");
    AddWeaponByName("weapon_shotgun");
    AddWeaponByName("weapon_tommy");
    AddWeaponByName("weapon_sniper");

    SwitchWeaponOffhand("weapon_cane");

    cameraRotation.y = Rotation.y;

}

void Player::UpdateWalkMovement(vec2 input)
{

    vec3 right = MathHelper::GetRightVector(Camera::rotation);

    vec3 forward = MathHelper::GetForwardVector(vec3(0, Camera::rotation.y, 0));

    if (freeFly)
        forward = Camera::Forward();

    vec3 movement = input.x * right + input.y * forward;

    //if (stepDelay.Wait())
        //movement = stepForceWalkDirection;




    velocity = controller.GetVelocity();


    if (OnGround())
    {

        bobProgress += length(MathHelper::XZ(velocity)) * Time::DeltaTime;

        /*
        TryStep(movement * 0.8f);

        TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), 5));
        TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), -5));

        TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), 10));
        TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), -10));

        TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), 20));
        TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), -20));

        TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), 35));
        TryStep(MathHelper::RotateVector(movement * 0.8f, vec3(0, 1, 0), -35));
        */
    }

    if (OnGround())
    {
        velocity = UpdateGroundVelocity(movement, velocity);
    }
    else
    {
        velocity = UpdateAirVelocity(movement, velocity);
    }


    velocity.y = controller.GetVelocity().y;

    if (freeFly)
        velocity = movement * 20.0f;

    controller.SetVelocity(velocity);

    if (OnGround()) 
    {
        if (Input::GetAction("jump")->Holding())
        {
            Jump();

        }
    }
}

void Player::UpdateBikeMovement(vec2 input)
{
    // Bike movement parameters
    const float maxSpeed = 15.0f;
    const float acceleration = 15.0f;
    const float lateralFriction = 1.5f;
    const float deltaTime = Time::DeltaTimeF; // Implement time handling

    vec3 moveRot = vec3(0, Camera::rotation.y - (input.x * 45.0 * 0), 0);

    // Get bike's forward direction (based on camera yaw)
    vec3 forward = MathHelper::GetForwardVector(moveRot);
    vec3 right = MathHelper::GetRightVector(moveRot);

    

    // Always move forward (override input)
    input = vec2(0, 1);
    vec3 movementDirection = input.x * right + input.y * forward;

    
    velocity = controller.GetVelocity();

    // Separate vertical and horizontal components
    float verticalVelocity = velocity.y;
    vec3 horizontalVelocity = vec3(velocity.x, 0.0f, velocity.z);

    // Calculate forward speed and lateral velocity
    float currentForwardSpeed = glm::dot(horizontalVelocity, forward);
    vec3 lateralVelocity = horizontalVelocity - (forward * currentForwardSpeed);

    // Apply forward acceleration with speed cap
    currentForwardSpeed = glm::min(currentForwardSpeed + acceleration * deltaTime, maxSpeed);

    // Apply lateral friction (drift effect)
    lateralVelocity *= glm::max(1.0f - lateralFriction * deltaTime, 0.0f);

    // Combine new velocity components
    vec3 newHorizontalVelocity = (forward * currentForwardSpeed) + lateralVelocity;
    vec3 newVelocity = vec3(newHorizontalVelocity.x, verticalVelocity, newHorizontalVelocity.z);

    // Update physics body velocity
    controller.SetVelocity(newVelocity);

    bikeMesh->Rotation.z = -dot(velocity, right)*2.5f;
    bikeMesh->Rotation.y -= bikeMesh->Rotation.z * 0.2f;


    AnimationPose pose = bikeMesh->GetAnimationPose();

    MathHelper::Transform frontRot = pose.GetBoneTransform("pelvis");
	MathHelper::Transform wheelRot = pose.GetBoneTransform("wheel_front");
    MathHelper::Transform rightArm = pose.GetBoneTransform("upperarm_r");

	frontRot.Rotation -= vec3(0, bikeMesh->Rotation.z * 1.0, 0);
	wheelRot.Rotation += vec3(Time::GameTime * 1000.0f, 0, 0);

    if (currentWeapon)
    {
        rightArm.Scale = vec3(0);
    }

	pose.SetBoneTransformEuler("pelvis", frontRot);
	pose.SetBoneTransformEuler("wheel_front", wheelRot);
    pose.SetBoneTransformEuler("upperarm_r", rightArm);

    bikeMesh->PasteAnimationPose(pose);
    bikeArmsMesh->PasteAnimationPose(bikeMesh->GetAnimationPose());
    bikeArmsMesh->Rotation = bikeMesh->Rotation;


    if (OnGround())
    {
        if (Input::GetAction("jump")->Holding())
        {
            Jump();

        }
    }

}

bool Player::CanSwitchSlot(int slot)
{
    if (!currentWeapon) return true;
    if (slot == currentSlot) return false;
    return currentWeapon->CanChangeSlot();
}

void Player::SwitchWeapon(const WeaponSlotData& data)
{
    DestroyWeapon();

    if (!data.className.empty())
    {
        currentWeapon = (Weapon*)Spawn(data.className);
		currentWeapon->owner = this;
        if (Level::Current->IsEntityTypeLoaded(data.className))
        {
            currentWeapon->LoadAssetsIfNeeded();
        }
        currentWeapon->Start();
        currentWeapon->SetData(data);
        //UpdateBody();
    }
}

void Player::SwitchWeaponOffhand(const string& classname)
{

    DestroyWeaponOffhand();

    if (!classname.empty())
    {
        currentOffhandWeapon = (Weapon*)Spawn(classname);
        currentOffhandWeapon->Start();
    }

}

void Player::DestroyWeaponOffhand()
{

    if (currentOffhandWeapon != nullptr)
    {

        currentOffhandWeapon->Destroy();
        currentOffhandWeapon = nullptr;

    }

}

void Player::SwitchToSlot(int slot, bool forceChange)
{
    if (!forceChange && !CanSwitchSlot(slot))
    {
        if (slot != currentSlot) // currentWeapon->IsMelee() && 
        {
            lastSlot = currentSlot;
            currentSlot = slot;
        }
        return;
    }

    if (slot < 0 || slot >= weaponSlots.size()) return;
    if (weaponSlots[slot].className.empty()) return;

    lastSlot = currentSlot;
    currentSlot = slot;
    SwitchWeapon(weaponSlots[slot]);
}

void Player::SwitchToMeleeWeapon(bool forceChange)
{
    if (!forceChange && currentWeapon && currentWeapon->IsMelee())
        return;

    if (!forceChange && currentWeapon && !currentWeapon->CanChangeSlot())
        return;

    if (!meleeWeapon.className.empty())
    {
        currentSlot = -1;
        SwitchWeapon(meleeWeapon);
    }
}

void Player::AddWeapon(const WeaponSlotData& weaponData)
{
    int slot = weaponData.slot;

    if (slot < 0 || slot >= weaponSlots.size()) return;

    if (weaponSlots[slot].className.empty() ||
        weaponSlots[slot].priority < weaponData.priority)
    {
        weaponSlots[slot] = weaponData;

        if (currentSlot == slot)
        {
            SwitchToSlot(slot, true);
        }
    }
}

void Player::AddWeaponByName(const string& className)
{

    Weapon* weap = (Weapon*)LevelObjectFactory::instance().create(className);

    AddWeapon(weap->GetDefaultData());

    delete(weap);

}

void Player::CreateWeapon(const string& className)
{

    Weapon* weap = (Weapon*)Spawn(className);

    weap->Start();
    weap->LoadAssetsIfNeeded();

    currentWeapon = weap;

}

void Player::DestroyWeapon()
{
    if (currentWeapon)
    {
        currentWeapon->Destroy();
        currentWeapon = nullptr;
    }
}

vec3 Player::GetBobForMainWeapon()
{
    
    vec3 bobT = vec3(0);

    bobT.y = (float)(sin(bobProgress * bobSpeed * 2) + 0.2f) * -0.15f;
    bobT.x = (float)((sin(bobProgress * bobSpeed * 1)) - 0.15f) * 0.3f;

    return bobT * 0.02f;
}

void Player::UpdateWeapon()
{

    vec3 bob = GetBobForMainWeapon();

    vec3 forwardOffset = Camera::Forward() * Camera::rotation.x * 0.01f;

    observationTarget->tags = { "player" };

    if (violanceCrimeActiveDelay.Wait())
    {
        observationTarget->tags.insert("violentCrime");
    }

    if (currentWeapon != nullptr)
    {

        if (Input::GetAction("test")->Pressed())
        {
			auto firearm = dynamic_cast<WeaponFirearm*>(currentWeapon);

            if (firearm)
            {
                firearm->SetAkimbo(!firearm->akimbo);
            }

        }

        vec3 relativeWeaponPos = vec3();
            
        vec3 currentWeaponRunRotation = lerp(vec3(), weaponRunRotation, RunProgress);

        vec3 rotatedWeaponPos = MathHelper::RotateAroundPoint(relativeWeaponPos, runRotatePoint, currentWeaponRunRotation);

        glm::quat qCurrent = MathHelper::GetRotationQuaternion(lerp(cameraRotation, Camera::rotation, 0.75f));
        glm::quat qAdd = MathHelper::GetRotationQuaternion(currentWeaponRunRotation);

        glm::quat qResult = qCurrent * qAdd;

		rotatedWeaponPos -= mix(vec3(), vec3(-0.05f, 0.02, 0.05), RunProgress);

        vec3 scaledBob = bob * mix(vec3(1),vec3(2.5,2.2f,2.2f), RunProgress);

        currentWeapon->HideWeapon = (currentOffhandWeapon != nullptr) ? 1.0f : bike_progress;
        currentWeapon->Position = MathHelper::TransformVector(rotatedWeaponPos, Camera::GetMatrix()) + MathHelper::TransformVector(scaledBob, Camera::GetRotationMatrix()) * currentWeapon->bobScale;
        currentWeapon->Rotation = MathHelper::ToYawPitchRoll(qResult);// +vec3(40.0f, 30.0f, 30.0f) * bike_progress;

        if (currentWeapon->Illegal)
        {
            observationTarget->tags.insert("illegal_weapon");
        }

    }

    if (currentOffhandWeapon != nullptr)
    {

        currentOffhandWeapon->Position = Camera::position + MathHelper::TransformVector(vec3(0, -bob.y+0.001, bob.x)*2.0f, Camera::GetRotationMatrix());
        currentOffhandWeapon->Rotation = lerp(cameraRotation, Camera::rotation, 0.3f);

    }

}

char* debug_level_name = new char[100];

void Player::UpdateDebugUI()
{

    auto draw = ImGui::GetForegroundDrawList();

    string fps = "fps: " + to_string((int)(1.0 / Time::DeltaTimeNoTimeScale));

    draw->AddText(NULL, 24.0f, ImVec2(10, 10), IM_COL32(255, 255, 255, 255), fps.c_str());

    if (EngineMain::MainInstance->Paused == false) return;

    ImGui::Begin("navigation");

    ImGui::Checkbox("draw nav mesh", &NavigationSystem::DebugDrawNavMeshEnabled);

    if (ImGui::Button("PlaceObstacle"))
    {
        NavigationSystem::RemoveObstacle(playerObstacle);
        playerObstacle = NavigationSystem::CreateObstacleBox(Position - vec3(1, 1, 1), Position + vec3(1, 1, 1));
    }

    if (ImGui::Button("place start location"))
    {
        testStart = Position;
        DebugDraw::Line(Position, Position - vec3(0, 1, 0), 2, 0.1);
    }

    if (ImGui::Button("calculate path to player"))
    {
        auto path = NavigationSystem::FindSimplePath(Position, testStart);

        path.insert(path.begin(), Position);

        DebugDraw::Path(path, 15, 0.05);
    }

    ImGui::End();

    ImGui::Begin("weapon");
    ImGui::DragFloat3("weaponRotationPoint", &runRotatePoint.x, 0.01);
    ImGui::DragFloat3("weaponRotation", &weaponRunRotation.x, 0.01);
    ImGui::End();

    ImGui::Begin("graphic");
    ImGui::SliderInt("multisample count",&EngineMain::MainInstance->MainRenderer->MultiSampleCount,0,8);
    ImGui::SliderFloat("resolution scale", &EngineMain::MainInstance->MainRenderer->ResolutionScale, 0, 10);
    ImGui::End();

    ImGui::Begin("debug");
    
	ImGui::DragFloat("time scale", &Time::TimeScale, 0.01f, 0.f, 3);
    ImGui::Checkbox("fixed simulation tick rate", &Time::SimulationLikeFixedTimeStep);

    if (ImGui::Checkbox("fly", &freeFly))
    {
        if (freeFly)
        {
            controller.SetCollisionMask(BodyType::None);
            controller.SetCollisionMask(BodyType::None);
        }
        else
        {
            controller.SetCollisionMask(BodyType::GroupCollisionTest);
            controller.SetCollisionMask(BodyType::CharacterCapsule);
        }
    }

    ImGui::InputText("level name", debug_level_name, 100);
    ImGui::SameLine();
    if (ImGui::Button("load"))
    {
        Level::LoadLevelFromFile(string(debug_level_name));
    }

    ImGui::Checkbox("draw physics", &Physics::DebugDraw);

    if (ImGui::Button("spawn guard npc"))
    {
        Entity* entity = Spawn("npc_guard");
        entity->Position = Camera::position + Camera::Forward() * 2.0f;
        entity->Start();
    }

    if (ImGui::Button("spawn civilian npc"))
    {
        Entity* entity = Spawn("npc_civilian");
        entity->Position = Camera::position + Camera::Forward() * 2.0f;
        entity->Start();
    }

    if (ImGui::Button("test serialization"))
    {

        json jPlayer;
        Serialize(jPlayer);

        serializedPlayer = jPlayer.dump(4);

        Logger::Log(serializedPlayer);

    }

    if (ImGui::Button("test DEserialization"))
    {

        //json jPlayer = json::parse(serializedPlayer);
        //Deserialize(jPlayer);

        //Logger::Log(jPlayer.dump(4));

    }

    ImGui::End();



}

bool Player::OnGround()
{
    return (coyoteTime.Wait() || afterStepDelay.Wait()) && jumpDelay.Wait() == false;
}

void Player::PerformAttack()
{


}

void Player::TryStep(vec3 dir)
{
    return;
    /*
    if (stepDelay.Wait()) return;

    vec3 pos = Position + dir/1.3f;

    if (pos == vec3())
        return;

    auto hit = Physics::LineTrace(pos, (pos - vec3(0, 0.85f, 0)), Physics::GetCollisionMask(LeadBody), {LeadBody});

    if (hit.hasHit == false)
        return;

    DebugDraw::Line(hit.position, hit.position + hit.normal);
    if (hit.normal.y < 0.9)
        return;



    vec3 hitPoint = hit.position;

    if (hitPoint == vec3())
        return;

    if (Physics::LineTrace(hitPoint + vec3(0, 0.05, 0), Position - vec3(0, 0.87f, 0), Physics::GetCollisionMask(LeadBody), { LeadBody }).hasHit == false)
    {
        return;
    }



    if (hitPoint.y > Position.y - 1 + 0.8f)
        return;

    if (Physics::SphereTrace(hitPoint + vec3(0,1,0) * 0.33f, hitPoint + vec3(0, 1, 0), 0.3f, Physics::GetCollisionMask(LeadBody), { LeadBody }).hasHit)
        return;

    if (Physics::SphereTrace(Position, Position + normalize(dir) * 0.2f, 0.3f, Physics::GetCollisionMask(LeadBody), { LeadBody }).hasHit)
        return;

    if (distance(hitPoint, Position) > 1.4)
        return;

    hit = Physics::LineTrace(Position, mix(Position, hitPoint, 1.1f) + vec3(0,1,0) * 0.2f, Physics::GetCollisionMask(LeadBody), {LeadBody});

    if (hit.hasHit)
    {

        return;
    }


    vec3 lerpPose = mix(Position, hitPoint, 0.0f);

    lerpPose.y = hitPoint.y + 1;

    float newOffset = Position.y - lerpPose.y;

    cameraHeightOffset += newOffset;
    Position.y -= newOffset;

    controller.SetPosition(lerpPose);

    Physics::SetBodyPosition(LeadBody,lerpPose);
	//DebugDraw::Line(lerpPose - vec3(0, 0.9f, 0), lerpPose + vec3(0, 1, 0), 10, 0.1f);

    //stepForceWalkDirection = normalize(MathHelper::XZ(hitPoint - Position));

    stepDelay.AddDelay(0.05f);
    afterStepDelay.AddDelay(0.1f);
    */
}

void Player::Update()
{

    //printf("%i \n",SkeletalMesh::skelMeshes);

    if(Input::GetAction("cameraView")->Pressed())
    {
		ThirdPersonView = !ThirdPersonView;
	}


	if (teleported == false && freeFly == false)
	{

        vec3 dir = normalize(controller.GetPosition() - oldPos);


		auto hit = Physics::LineTrace(oldPos - dir * 0.2f, controller.GetPosition(), BodyType::World | BodyType::WorldSkybox);

		if (hit.hasHit)
		{
			controller.SetPosition(hit.position - dir * 0.3f);

		}

		hit = Physics::SphereTrace(oldPos - dir * 0.1f, controller.GetPosition(), 0.2f, BodyType::World | BodyType::WorldSkybox);

		if (hit.hasHit)
		{
			controller.SetPosition(hit.shapePosition);

		}


	}
	teleported = false;
    oldPos = controller.GetPosition();

    Position = controller.GetSmoothPosition();


    if (controller.onGround)
    {
        coyoteTime.AddDelay(0.1f);
    }


    if (Input::LockCursor)
    {

		float fovScale = Camera::FOV / 75.0f;

		//fovScale = mix(fovScale, 1.0f, 0.5f);

        cameraRotation.y += Input::MouseDelta.x * fovScale;
        cameraRotation.x -= Input::MouseDelta.y * fovScale;

        vec2 touchMovement = Hud.ScreenControls->TouchArea->GetTouchMovement();

        touchMovement /= -5.0;

        cameraRotation.y += touchMovement.x * fovScale;
        cameraRotation.x -= touchMovement.y * fovScale;

        cameraRotation.x = glm::clamp(cameraRotation.x, -80.0f,80.0f);

        if (on_bike)
        {
            cameraRotation.x = glm::clamp(cameraRotation.x, -50.0f, 59.0f);
        }

    }


    vec2 input = Input::GetLeftStickPosition();

    input += Hud.ScreenControls->Joystick->InputPosition;

    if (Input::GetAction("forward")->Holding())
        input += vec2(0, 1);

    if (Input::GetAction("backward")->Holding())
        input += vec2(0, -1);

    if (Input::GetAction("left")->Holding())
        input += vec2(-1, 0);

    if (Input::GetAction("right")->Holding())
        input += vec2(1, 0);

    if (length(input) > 1)
        input = normalize(input);

	if (false)
	{

		if (Input::GetAction("dash")->Holding() && input.y > 0.4f && OnGround())
		{
			RunProgress += Time::DeltaTimeF * 4.0f;
		}
		else
		{
			RunProgress -= Time::DeltaTimeF * 4.0f;
		}
	}

    RunProgress = std::clamp(RunProgress, 0.0f, 1.0f);

    //maxSpeed = mix(WalkSpeed, RunSpeed, RunProgress);

    if (on_bike == false)
    {
        UpdateWalkMovement(input);
    }

    if (on_bike)
    {
        bike_progress += Time::DeltaTimeF * 3;
    }
    else
    {
        bike_progress -= Time::DeltaTimeF * 3;
    }

    bike_progress = glm::clamp(bike_progress, 0.0f, 1.0f);

    bikeMesh->Position = Position - vec3(0, 0.9f - 0.8f, 0);
    bikeMesh->Rotation = vec3(0, cameraRotation.y, 0);
    bikeMesh->Update();
    if (on_bike)
    {
        UpdateBikeMovement(input);
    }
    bikeArmsMesh->Rotation = bikeMesh->Rotation;
    bikeArmsMesh->Position = bikeMesh->Position;
    bikeArmsMesh->PasteAnimationPose(bikeMesh->GetAnimationPose());

    vec3 playerForward = MathHelper::GetForwardVector(vec3(0, cameraRotation.y, 0));

	cameraHeightOffset = mix(cameraHeightOffset, 0.0f, Time::DeltaTimeF * 5.0f);
    Camera::position = Position + vec3(0, 0.7, 0) - vec3(0,0.25f,0) * bike_progress + vec3(0,1,0) * cameraHeightOffset + playerForward*0.1f;

    

    Camera::rotation = cameraRotation;

    vec3 right = MathHelper::GetRightVector(Camera::rotation);

    Camera::rotation.z = -dot(velocity, right) * mix(-0.2f, 0.3f, bike_progress);

    if (InThirdPerson() == false)
    {
        UpdateBody();
    }



    if (Input::GetAction("bike")->Holding() && OnGround())
    {
        StartBike();
    }
    else
    {
        StopBike();
    }

    bool dashEnded = false;

    if (wasDashing && dashProgress.Wait() == false)
    {
        dashEnded = true;
    }

    wasDashing = false;
    if (dashProgress.Wait())
    {
        controller.SetVelocity(dashVector);
        wasDashing = true;
    }
    else
    {

        if (dashEnded)
        {
            controller.SetVelocity(normalize(dashVector) * Speed);
        }

        if (Input::GetAction("dash")->Pressed())
        {

            vec3 dashDir = right * input.x + playerForward * input.y;

            if (length(input) < 0.1)
            {
                dashDir = playerForward;
            }

            dashVector = dashDir * 20.0f;

            dashProgress.AddDelay(0.2f);
        }
    }



    if (Input::GetAction("qSave")->Pressed())
    {
        LevelSaveSystem::SaveLevelToFile("quicksave");
    }

    if (Input::GetAction("qLoad")->Pressed())
    {
        LevelSaveSystem::LoadLevelFromFile("quicksave");
    }

    if (currentWeapon != nullptr)
    {
        if (currentWeapon->Data.slot != currentSlot)
        {
            if (currentWeapon->CanChangeSlot())
            {
                SwitchWeapon(weaponSlots[currentSlot]);
            }
            
        }
    }
    else
    {
        SwitchToSlot(currentSlot);
    }

    if (Input::GetAction("slotMelee")->Pressed())
        SwitchToMeleeWeapon();

    if (Input::GetAction("slot1")->Pressed())
        SwitchToSlot(0);

    if (Input::GetAction("slot2")->Pressed())
        SwitchToSlot(1);

    if (Input::GetAction("slot3")->Pressed())
        SwitchToSlot(2);

    if (Input::GetAction("slot4")->Pressed())
        SwitchToSlot(3);

    if (Input::GetAction("slot5")->Pressed())
        SwitchToSlot(4);

    if (Input::GetAction("lastSlot")->Pressed())
        SwitchToSlot(lastSlot);

}

void Player::AsyncUpdate()
{

    if (InThirdPerson())
    {
        UpdateBody();
    }

    controller.Update(Time::DeltaTimeF);
    bodyAnimator.Update();

    UpdateCurrentRestrictedArea();

}

void Player::LateUpdate()
{

    if(EngineMain::MainInstance->Paused == false)
    if (Input::GetAction("pause")->Pressed())
    {
        EngineMain::MainInstance->Paused = !EngineMain::MainInstance->Paused;

        Input::LockCursor = !EngineMain::MainInstance->Paused;

    }

    Input::LockCursor = !EngineMain::MainInstance->Paused;

    UpdateWeapon();

    if (CurrentMaxRestrictionLevel > CurrentClearance)
    {
        observationTarget->tags.insert("trespassing");
    }

    Hud.Update();

}

void Player::UpdateThirdPersonCamera()
{

	vec3 forward = normalize(MathHelper::XZ(Camera::Forward()));

    Camera::position = Position;
	Camera::position -= forward * 0.1f;

    vec3 startPos = Position + vec3(0,1,0);

    vec3 targetCameraPos = Camera::position + vec3(0,0.4f,0);

    targetCameraPos += Camera::Forward() * -2.4f;
    targetCameraPos += Camera::Up() * 0.7f;
	//targetCameraPos += Camera::Right() * 0.05f;

    auto hit = Physics::SphereTrace(startPos, targetCameraPos, 0.3f, BodyType::GroupCollisionTest, {}, {this});
    if (hit.hasHit)
    {
        Camera::position = hit.shapePosition;
    }
    else
    {
        Camera::position = targetCameraPos;
	}

}

void Player::UpdateBody()
{

    if (EngineMain::MainInstance->SimulatingGameTicks) return;

    bodyAnimator.movementSpeed = length(MathHelper::XZ(velocity));

    vec3 playerForward = MathHelper::GetForwardVector(vec3(0,cameraRotation.y,0));

    auto pose = bodyAnimator.GetResultPose();
    //pose.SetBoneTransform();

    if(InThirdPerson())
    {
        if (currentWeapon)
        {

            pose = currentWeapon->ApplyWeaponAnimation(pose);

        }
    }
    else
    {
        mat4 scale0 = scale(vec3(0));
        //pose.SetBoneTransform("neck_01", scale0);
        pose.SetBoneTransform("upperarm_r", scale0);
        pose.SetBoneTransform("upperarm_l", scale0);
    }


    bodyMesh->PasteAnimationPose(pose);
    bodyMesh->Position = Position - vec3(0, controller.height / 2.0f,0) - playerForward*0.3f;
    bodyMesh->Rotation.y = cameraRotation.y;

    if (InThirdPerson())
    {
        bodyMesh->Position = Position - vec3(0, controller.height / 2.0f, 0);
    }


    //std::unordered_map<std::string, mat4> poseT;
    //poseT["thigh_r"] = translate(Camera::position + Camera::Forward()) * scale(vec3(0.01f));
    //bodyMesh->ApplyWorldSpaceBoneTransforms(poseT);

    Camera::ApplyCameraShake(Time::DeltaTimeF);

    if (InThirdPerson())
    {
		UpdateThirdPersonCamera();
    }
    else
    {
        Camera::position = MathHelper::DecomposeMatrix(bodyMesh->GetBoneMatrixWorld("head")).Position + playerForward * 0.3f;

    }



    observationTarget->position = Position + vec3(0,0.65f,0);


    Physics::SetBodyPosition(hitbox, Camera::position - vec3(0,0.5f,0));

}

bool Player::InThirdPerson()
{

    if (currentWeapon)
    {
        return ThirdPersonView && !currentWeapon->ForceFirstPerson;
    }

    return ThirdPersonView;
}

void Player::Serialize(json& target)
{

    Entity::Serialize(target);

    SERIALIZE_FIELD(target, cameraRotation);
    SERIALIZE_FIELD(target, velocity);

}

void Player::OnDamage(float Damage, Entity* DamageCauser, Entity* Weapon)
{
    Entity::OnDamage(Damage, DamageCauser, Weapon);
}

void Player::OnPointDamage(float Damage, vec3 Point, vec3 Direction, string bone, Entity* DamageCauser, Entity* Weapon)
{
    Entity::OnPointDamage(Damage, Point, Direction, bone, DamageCauser, Weapon);

    //GlobalParticleSystem::SpawnParticleAt("hit_flesh", Point - vec3(0,0.5f,0), MathHelper::FindLookAtRotation(vec3(0), -Direction - vec3(0, 1, 0)), vec3(0.2f));

}

void Player::UpdateCurrentRestrictedArea()
{

    int currentAreaLevel = 0;

    auto results = Physics::PointTrace(Position, BodyType::Area1);

    for (auto result : results)
    {

        RestrictedArea* area = dynamic_cast<RestrictedArea*>(result.entity);

        if (area == nullptr) continue;

        if (area->RestrictionLevel > currentAreaLevel)
            currentAreaLevel = area->RestrictionLevel;

    }

    CurrentMaxRestrictionLevel = currentAreaLevel;

}

void Player::Deserialize(json& source)
{

    Entity::Deserialize(source);

    DESERIALIZE_FIELD(source, cameraRotation);
    DESERIALIZE_FIELD(source, velocity);

    controller.SetVelocity(velocity);
    Teleport(Position);


}

void Player::Teleport(vec3 target)
{

    teleported = true;

    Position = target;
    oldPos = Position;

    controller.SetPosition(target);
    controller.heightSmoothOffset = 0;
    

}

void Player::MoveTo(vec3 target)
{

    auto hit = Physics::SphereTrace(Position, target, 0.2f, BodyType::World | BodyType::WorldSkybox);

    if (hit.hasHit)
    {

        target = hit.shapePosition;

    }

    Position = target;

    controller.SetPosition(target);
    controller.heightSmoothOffset = 0;

}

void Player::StartBike()
{
    if (on_bike) return;

    bikeMesh->PlayAnimation("draw", true, 0.7f);
    on_bike = true;
}

void Player::StopBike()
{
    if (on_bike == false) return;

    bikeMesh->PlayAnimation("hide", true, 0.7f);
    on_bike = false;
}

void Player::ToggleBike()
{
    if (on_bike)
    {
        StopBike();
    }
    else
    {
        StartBike();
    }
}

void Player::LoadAssets()
{
    bikeMesh->LoadFromFile("GameData/models/player/bike/bike.glb");
    bikeMesh->TexturesLocation = "GameData/models/player/bike/textures/";
    bikeMesh->PreloadAssets();
    bikeMesh->PlayAnimation("hide",true);

    bikeArmsMesh->LoadFromFile("GameData/arms.glb");
    bikeArmsMesh->PreloadAssets();

    bodyMesh->LoadFromFile("GameData/models/player/body/player_body.glb");
    //bodyMesh->LoadFromFile("GameData/models/npc/guard.glb/");
    bodyMesh->DepthPrePath = false;
    bodyMesh->Masked = true;
    //bodyMesh->TexturesLocation = "GameData/models/npc/guard.glb/";
    bodyMesh->PreloadAssets();

    bodyAnimator.LoadAssetsIfNeeded();

}
