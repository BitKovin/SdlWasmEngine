#pragma once
#include "../Entity.h"
#include "../StaticMesh.h"

class StaticMeshEntity : public Entity
{
public:
	
	StaticMesh* mesh;

	void FromData(EntityData data)
	{
		Entity::FromData(data);

		mesh = new StaticMesh(this);

		mesh->LoadFromFile("GameData/" + data.GetPropertyString("model"));

		vec3 angles = data.GetPropertyVectorRotation("angles");

		float scale = data.GetPropertyFloat("modelscale", 32.0f);

		vec3 scaleVec = data.GetPropertyVectorPosition("modelscale_vec")*32.0f;
		scaleVec.z *= -1;
		if (scaleVec == vec3())
		{
			scaleVec = vec3(scale);
		}

		mesh->Rotation = EntityData::ConvertRotation(angles);
		mesh->Position = Position;
		mesh->Scale = scaleVec / 32.0f;

		mesh->Brightness = 2;

		mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/" + data.GetPropertyString("texture"));

		Drawables.push_back(mesh);
	}

private:

};