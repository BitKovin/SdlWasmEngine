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
				auto b = m->GetBoundingBox();

				boundsPoints.push_back(b.Min);
				boundsPoints.push_back(b.Max);

			}

		}

		initialBounds = BoundingBox::FromPoints(boundsPoints);

	}

	void AsyncUpdate()
	{

		Entity::AsyncUpdate();

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

private:

};

