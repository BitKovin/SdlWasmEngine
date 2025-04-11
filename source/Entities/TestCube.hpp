#pragma once

#include "../Entity.hpp"

#include "../Input.h"

#include "../MathHelper.hpp"

#include "../Camera.h"

#include "../SkeletalMesh.hpp"
#include "../AssetRegisty.h"

class TestCube : public Entity
{
public:

	StaticMesh* mesh;

	TestCube()
	{
		mesh = new StaticMesh();
	}

	TestCube(vec3 pos)
	{
		Position = pos;

		mesh = new StaticMesh();

		Start();

	}

	~TestCube()
	{

	}



	void Start()
	{
		mesh->LoadFromFile("GameData/cube.obj");
		mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");

		Drawables.push_back(mesh);

		LeadBody = Physics::CreateBoxBody(this, Position, vec3(1), 10, false);

	}

	void Update()
	{
		mesh->Position = Position;
		mesh->Rotation = Rotation;
	}


private:

};