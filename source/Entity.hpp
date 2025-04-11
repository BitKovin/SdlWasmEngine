#pragma once

#include <vector>

#include "EObject.hpp"
#include "LevelObject.hpp"

#include "IDrawMesh.h"

#include "glm.h"

#include "Physics.h"

#include "RegisterLevelObject.h"

#include "MapData.h"

using namespace std;

class Entity : public LevelObject
{
public:

	vec3 Position = vec3();

	vec3 Rotation = vec3();

	vector<IDrawMesh*> Drawables;

	Body* LeadBody = nullptr;

	vector<Body*> Bodies;

	string ClassName = "Entity";

	string Name = "";


	Entity()
	{

	}
	~Entity()
	{

	}

	void UpdatePhysics()
	{
		if (LeadBody)
		{
			Position = FromPhysics(LeadBody->GetPosition());
			Rotation = MathHelper::ToYawPitchRoll(FromPhysics(LeadBody->GetRotation()));
		}
	}

	virtual void FromData(EntityData data)
	{

		Name = data.GetPropertyString("targetName");

		Position = data.GetPropertyVectorPosition("origin");

	}

	void Finalize()
	{
		for (IDrawMesh* mesh : Drawables)
		{
			mesh->FinalizeFrameData();
		}
	}

	vector<IDrawMesh*> GetDrawMeshes()
	{
		return Drawables;
	}

	void DestroyDrawables()
	{
		for (IDrawMesh* mesh : Drawables)
		{
			delete(mesh);
		}

		Drawables.clear();
	}

	void virtual Destroy()
	{
		DestroyDrawables();
		Physics::DestroyBody(LeadBody);
		LeadBody = nullptr;
		for (Body* body : Bodies)
		{
			Physics::DestroyBody(body);
		}
		Bodies.clear();
	}

protected:

	void OnDispose()
	{
		Destroy();
	}

private:

};