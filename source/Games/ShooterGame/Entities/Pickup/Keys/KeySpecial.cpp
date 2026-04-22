#include "PickupKeyBase.h"

class KeySpecial : public KeyBase
{
public:
	KeySpecial()
	{
		keyType = DoorKey::Special;
	}

	void LoadAssets() override
	{
		PickupBase::LoadAssets();
		auto mesh = new StaticMesh(this);
		mesh->LoadFromFile("GameData/models/pickups/key.obj");
		mesh->TexturesLocation = "GameData/models/pickups/";
		mesh->Position = Position;
		mesh->Rotation = Rotation;
		mesh->Scale = vec3(1 / 32.f);
		mesh->MeshCustomShaderParams["rim_pow"] = vec4(2.0f);
		mesh->MeshCustomShaderParams["rim_color"] = vec4(1.8f, 0.2f, 2.0f, 1.0f);

		Drawables.push_back(mesh);
	}

private:

};

REGISTER_ENTITY(KeySpecial, "item_key_special")
