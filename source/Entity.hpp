#pragma once

#include <vector>

#include "EObject.hpp"
#include "LevelObject.hpp"

#include "IDrawMesh.h"

#include "glm.h"

#include "Physics.h"

using namespace std;

class Entity : public LevelObject
{
public:

	vec3 Position = vec3();

	vec3 Rotation = vec3();

	vector<IDrawMesh*> Drawables;

	Body* LeadBody = nullptr;

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
			mesh->Dispose();
			free(mesh);
		}
	}

	void virtual Destroy()
	{
		DestroyDrawables();
	}

private:

};