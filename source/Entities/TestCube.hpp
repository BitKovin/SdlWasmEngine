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
	TestCube()
	{

	}

	TestCube(vec3 pos)
	{
		Position = pos;

		Start();

	}

	~TestCube()
	{

	}

	SkeletalMesh mesh;

	Body* body;

	void Start()
	{
		mesh.LoadFromFile("GameData/cube.obj");
		mesh.ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");

		Drawables.push_back(&mesh);

		body = Physics::CreateBoxBody(this ,Position, vec3(1), 10, false);

		LeadBody = body;

	}

	void Update()
	{
		mesh.Position = Position;
		mesh.Rotation = Rotation;
	}

	void Destroy()
	{

		Physics::DestroyBody(body);

		Entity::Destroy();
	}


private:

};