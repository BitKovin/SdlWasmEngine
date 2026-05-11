#pragma once

#include <Entity.h>

class DynamicPhysicsBrush : public Entity
{
public:
	DynamicPhysicsBrush() : Entity()
	{
		ConvexCollision = true;
		Static = false;
		DefaultBrushGroup = BodyType::MainBody;
		DefaultBrushCollisionMask = BodyType::GroupCollisionTest;
		SaveGame = true;
	}

	BoundingBox initialBounds;

	void Start()
	{

		Entity::Start();

		LeadBody->SetMotionType(JPH::EMotionType::Dynamic);

		vector<vec3> boundsPoints;

		for (auto model : Drawables)
		{
			BSPModelRef* m = (BSPModelRef*)model;

			if (m)
			{

				m->Static = false;

				auto b = m->GetBoundingBox();

				boundsPoints.push_back(b.Min);
				boundsPoints.push_back(b.Max);
				
			}

		}

		initialBounds = BoundingBox::FromPoints(boundsPoints);

	}

	void AsyncUpdate()
	{


		for (auto model : Drawables)
		{
			BSPModelRef* m = (BSPModelRef*)model;

			if (m)
			{
				m->Position = Position;
				m->Rotation = Rotation;
			}

		}

	}

	void Serialize(json& target)
	{
		Entity::Serialize(target);

		vec3 velocity = FromPhysics(LeadBody->GetLinearVelocity());
		vec3 angularVelocity = FromPhysics(LeadBody->GetAngularVelocity());

		SERIALIZE_FIELD(target, velocity);
		SERIALIZE_FIELD(target, angularVelocity);


	}

	void Deserialize(json& source)
	{
		Entity::Deserialize(source);
		
		vec3 velocity = vec3();
		vec3 angularVelocity = vec3();

		DESERIALIZE_FIELD(source, velocity);
		DESERIALIZE_FIELD(source, angularVelocity);

		Physics::SetBodyPositionAndRotation(LeadBody, Position, Rotation);
		Physics::SetLinearVelocity(LeadBody, velocity);
		Physics::SetAngularVelocity(LeadBody, angularVelocity);

	}

private:

};

