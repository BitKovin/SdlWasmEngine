#pragma once

#include <Entity.h>
#include <Navigation/Navigation.hpp>
#include <imgui/imgui.h>

#include <InteractionSystem/IInteractive.h>

class Door : public Entity , public IInteractive
{
public:

	Door()
	{
		SaveGame = true;
	}

	StaticMesh* doorMesh = nullptr;
	StaticMesh* doorFrameMesh = nullptr;

	float DoorRotation = 0.0f;

	bool IsOpen = false;

	vec2 doorSize = vec2(1.3, 2.45);

	bool Dirty = false;

	dtObstacleRef obstacleRef;

	Body* npcDetectionBody = nullptr;

	bool pendingClose = false;
	Delay pendingCloseDelay = Delay();

	void FromData(EntityData data)
	{
		Entity::FromData(data);

		LoadAssetsIfNeeded();

		Rotation.y = data.GetPropertyFloat("angle");

		doorFrameMesh->Position = Position;
		doorFrameMesh->Rotation = Rotation;
		doorMesh->Position = Position;
		doorMesh->Rotation = Rotation;
	}

	void Start() override;
	
	void UpdateDebugUI() override
	{
		Entity::UpdateDebugUI();
		ImGui::Begin("Door Debug UI");
		ImGui::SliderFloat("Door Rotation", &DoorRotation, -110.0f, 110.0f);
		ImGui::End();
	}

	void UpdatePhysics() override
	{
	}

	void Update();

	void LoadAssets();

	void Serialize(json& target) override
	{
		Entity::Serialize(target);
		SERIALIZE_FIELD(target, DoorRotation);
		SERIALIZE_FIELD(target, targetDoorRotation);
		SERIALIZE_FIELD(target, IsOpen);
		SERIALIZE_FIELD(target, pendingClose);
		SERIALIZE_FIELD(target, pendingCloseDelay);
	}

	void Deserialize(json& source) override
	{
		Entity::Deserialize(source);
		DESERIALIZE_FIELD(source, DoorRotation);
		DESERIALIZE_FIELD(source, targetDoorRotation);
		DESERIALIZE_FIELD(source, IsOpen);
		DESERIALIZE_FIELD(source, pendingClose);
		DESERIALIZE_FIELD(source, pendingCloseDelay);

		Dirty = true;
	}

	void OpenFromPosition(vec3 requesterPosition);
	void CloseDoor();

	void OnBodyEntered(Body* body, Entity* entity) override;
	void OnBodyExited(Body* body, Entity* entity) override;

	void Interact(Entity* interactingEntity) override;

private:

	float targetDoorRotation = 0.0f;
	float openSpeed = 90.0f;  // Degrees per second for opening/closing animation

};
