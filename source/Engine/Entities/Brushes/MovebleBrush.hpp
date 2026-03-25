#pragma once

#include "../../Entity.h"
#include "../transformPoint.h"

class MovebleBrush : public Entity
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

	MovebleBrush() : Entity()
	{
		SaveGame = true;

		Static = false;

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

	}

	void Start()
	{
		Entity::Start();

		Entity* offsetPoint = Level::Current->FindEntityWithName(offsetPointName);

		if (offsetPoint == nullptr) return;

		vec3 referenceRotation = EntityData::ConvertRotation(vec3(0, 0, 0), true);

		offsetPosition = offsetPoint->Position;
		offsetRotation = MathHelper::NormalizeAngles(offsetPoint->Rotation) - MathHelper::NormalizeAngles(referenceRotation);

		for (auto model : Drawables)
		{
			BSPModelRef* m = (BSPModelRef*)model;

			if (m)
			{
				m->Static = false;
			}

		}

	}

	void LateUpdate()
	{

		Entity::LateUpdate();

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

		oldProgress = progress;
		updateSleepDelay.AddDelay(0.2);

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

private:
	 
};
