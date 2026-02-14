#include "Door.h"

void Door::Start()
{
	Entity::Start();

	LeadBody = Physics::CreateBoxBody(this,Position, Scale * vec3(doorSize.x, doorSize.y, 0.2f), 10, false, BodyType::MainBody, BodyType::GroupCollisionTest | HitBox);

	npcDetectionBody = Physics::CreateBoxBody(this, Position, Scale * vec3(doorSize.x + 1.0f, doorSize.y + 2.0f, 1.0f), 1, true, BodyType::Area2, BodyType::CharacterCapsule);
    npcDetectionBody->SetIsSensor(true);

	Physics::SetMotionType(LeadBody, JPH::EMotionType::Kinematic);

}

void Door::Update()
{

	if (pendingClose && pendingCloseDelay.Wait() == false)
    {
        CloseDoor();
        pendingClose = false;
    }

    float deltaTime = Time::DeltaTimeF;
    float diff = targetDoorRotation - DoorRotation;

    if (diff != 0.0f)
    {
        float maxStep = openSpeed * deltaTime;
        float step = glm::sign(diff) * maxStep;

        // If taking the step would overshoot the target, clamp
        if (glm::abs(step) >= glm::abs(diff))
        {
            DoorRotation = targetDoorRotation; // snap
        }
        else
        {
            DoorRotation += step;
        }
    }

    // Update mesh transform
    doorMesh->Rotation.y = Rotation.y + DoorRotation;
    doorMesh->Position = MathHelper::RotateAroundPoint(
        Position,
        Position - MathHelper::GetRightVector(Rotation) * doorSize.x / 2.0f,
        vec3(0, DoorRotation, 0)
    );

    Physics::MoveKinematic(
        LeadBody,
        doorMesh->Position + vec3(0, doorSize.y / 2, 0),
        doorMesh->Rotation
    );

    if (Dirty)
    {
        NavigationSystem::RemoveObstacle(obstacleRef);

		if (IsOpen)
		{

			// Hinge world position (same as your code)
			vec3 hingePos = Position - MathHelper::GetRightVector(Rotation) * (doorSize.x * 0.5f);

			vec3 size = vec3(doorSize.x, doorSize.y, 0.2f);

			BoundingBox box(-size / 2.0f, size / 2.0f);

			vec3 fullOpenDoorPos = MathHelper::RotateAroundPoint(Position, Position - MathHelper::GetRightVector(Rotation) * doorSize.x / 2.0f, vec3(0, targetDoorRotation, 0));
			vec3 fullopenDoorRotation = Rotation + vec3(0, targetDoorRotation, 0);

			box = box.Transform(fullOpenDoorPos, fullopenDoorRotation);

            box.Min -= vec3(0.3);
			box.Max += vec3(0.3);

			obstacleRef = NavigationSystem::CreateObstacleBox(box.Min + vec3(0, 1, 0), box.Max + vec3(0, 1, 0));
		}
		Dirty = false;
    }


}

void Door::LoadAssets()
{

	doorMesh = new StaticMesh(this);
	doorMesh->LoadFromFile("GameData/models/enviroment/door.glb");
	doorMesh->TexturesLocation = "GameData/models/enviroment/door.glb/";
	doorMesh->MeshHideList.insert("frame");
    doorMesh->Brightness = 2;
	Drawables.push_back(doorMesh);

	doorFrameMesh = new StaticMesh(this);
	doorFrameMesh->LoadFromFile("GameData/models/enviroment/door.glb");
	doorFrameMesh->TexturesLocation = "GameData/models/enviroment/door.glb/";
	doorFrameMesh->MeshHideList.insert("door");
    doorFrameMesh->Brightness = 2;
	Drawables.push_back(doorFrameMesh);

}

void Door::OpenFromPosition(vec3 requesterPosition)
{
	vec3 toRequester = requesterPosition - Position;
	vec3 forward = MathHelper::GetForwardVector(Rotation);
	float dot = glm::dot(toRequester, forward);  // Assuming vec3 is glm::vec3
	float openAngle = 110.0f;  // Standard open angle; adjust if needed (e.g., to 110.0f)
	targetDoorRotation = (dot < 0) ? openAngle : -openAngle;

    IsOpen = true;

	Dirty = true;

    pendingClose = false;

}


void Door::CloseDoor()
{
	targetDoorRotation = 0.0f;
    IsOpen = false;

	Dirty = true;

    pendingClose = false;

}

void Door::OnBodyEntered(Body* body, Entity* entity)
{

    if (entity->HasTag("npc"))
    {

        Logger::Log("Npc entered door detection area\n");

        pendingClose = true;
		pendingCloseDelay.AddDelay(2.0f);
    }
}

void Door::OnBodyExited(Body* body, Entity* entity)
{

    if (entity->HasTag("npc"))
    {

		Logger::Log("Npc exited door detection area\n");

        pendingClose = true;
        pendingCloseDelay.AddDelay(1.0f);
    }
}

REGISTER_ENTITY(Door, "env_door")