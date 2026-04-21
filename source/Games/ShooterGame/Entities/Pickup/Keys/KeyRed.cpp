#include "PickupKeyBase.h"

class KeyRed : public KeyBase
{
public:
	KeyRed()
	{
		keyType = DoorKey::Red;
	}

	void LoadAssets() override
	{
		KeyBase::LoadAssets();
		auto mesh = new StaticMesh(this);
		mesh->LoadFromFile("GameData/models/pickups/key.obj");
		mesh->TexturesLocation = "GameData/models/pickups/";
		mesh->Position = Position;
		mesh->Rotation = Rotation - vec3(0, 90, 0);
		mesh->Scale = vec3(1 / 32.f);
		mesh->MeshCustomShaderParams["rim_pow"] = vec4(2.0f);
		mesh->MeshCustomShaderParams["rim_color"] = vec4(2.0f, 0.4f, 0.4f, 1.0f);

		Drawables.push_back(mesh);
	}

private:

};

REGISTER_ENTITY(KeyRed,"item_key_red")