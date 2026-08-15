#pragma once

#include "../transformPoint.h"
#include <Logger.hpp>
#include <Network/NetworkedEntity.h>

#include <Navigation/Navigation.hpp>

class MovebleBrush : public NetworkedEntity
{
public:
	
	vec3 targetLocation = vec3();

	float time = 1;

	float progress = 0;

	bool open = false;

	string offsetPointName = "";

	vec3 offsetPosition = vec3();
	vec3 offsetRotation = vec3();

	float oldProgress = 0;

	Delay updateSleepDelay;

	bool useLightmap = false;

	dtObstacleRef obstacleRef = 0;

	MovebleBrush() : NetworkedEntity()
	{
		SaveGame = true;
		ConvexCollision = true;
		Static = false;
		DefaultBrushCollisionMask = BodyType::GroupCollisionTest & ~BodyType::World;
	}

	void Serialize(json& target)
	{

		Entity::Serialize(target);

		SERIALIZE_FIELD(target, open);
		SERIALIZE_FIELD(target, progress);

	}

	void Deserialize(json& source)
	{

		Entity::Deserialize(source);

		DESERIALIZE_FIELD(source, open);
		DESERIALIZE_FIELD(source, progress);

	}

	void FromData(EntityData data)
	{

		Entity::FromData(data);

		time = data.GetPropertyFloat("time", time);

		offsetPointName = data.GetPropertyString("rotationPointName");

		targetLocation = data.GetPropertyVectorPosition("targetLocation");

		useLightmap = data.GetPropertyBool("useLightmap");

	}

	void UpdateObstacle();

	void Start()
	{
		Entity::Start();

		if (offsetPointName.empty() == false)
		{
			Entity* offsetPoint = Level::Current->FindEntityWithName(offsetPointName);

			if (offsetPoint == nullptr) return;

			vec3 referenceRotation = EntityData::ConvertRotation(vec3(0, 0, 0), true);

			offsetPosition = offsetPoint->Position;
			offsetRotation = MathHelper::NormalizeAngles(offsetPoint->Rotation) - MathHelper::NormalizeAngles(referenceRotation);
		}


		for (auto model : Drawables)
		{
			BSPModelRef* m = (BSPModelRef*)model;

			if (m)
			{
				m->Static = useLightmap;
			}

		}
	}

	void LateUpdate()
	{

		Entity::LateUpdate();

		if (obstacleRef == 0)
		{
			UpdateObstacle();
		}

		if (open)
		{
			progress += Time::DeltaTime / time;
		}
		else
		{
			progress -= Time::DeltaTime / time;
		}

		progress = glm::clamp(progress, 0.0f, 1.0f);

		if (progress == oldProgress && updateSleepDelay.Wait() == false)
		{
			return;
		}

		vec3 startPos = Position;

		Position = lerp(vec3(0.0f), targetLocation, progress);

		Rotation = lerp(vec3(0.0f), offsetRotation, progress);

		Position -= offsetPosition;


		quat rotationQuat = MathHelper::GetRotationQuaternion(offsetRotation);

		Rotation = MathHelper::ToYawPitchRoll(slerp(quat(1.f, 0.f, 0.f, 0.f), rotationQuat, progress));

		rotationQuat = MathHelper::GetRotationQuaternion(Rotation);

		Position = rotationQuat * Position;

		Position += offsetPosition;


		Physics::MoveKinematic(LeadBody, Position, Rotation);

		for (const auto& drawable : Drawables)
		{

			BSPModelRef* model = (BSPModelRef*)drawable;

			model->Position = Position;
			model->Rotation = Rotation;

		}

		UpdateObstacle();

		oldProgress = progress;
		updateSleepDelay.AddDelay(0.1);

	}

	void OnAction(string action) 
	{

		Logger::Log(action);

		if (action == "open")
		{
			open = true;		

		}
		else if (action == "close")
		{

			open = false;

		}

	}

	void NetSerialize(NetPacket& packet) override 
	{

		packet.WriteBool(open);
		packet.WriteFloat(progress);

	}
	void NetDeserialize(NetPacket& packet) override 
	{

		open = packet.ReadBool();

		float newProgress = packet.ReadFloat();

		if (newProgress > progress + 0.05f || newProgress < progress - 0.05f)
		{
			progress = newProgress;
		}

	}

private:
	 
};
