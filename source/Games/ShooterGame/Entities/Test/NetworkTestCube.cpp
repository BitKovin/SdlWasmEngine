#include <Network/NetworkedEntity.h>
#include <StaticMesh.h>

class NetworkTestCube : public NetworkedEntity
{
public:
	
	StaticMesh* mesh = nullptr;

	void Start() override
	{
		NetworkedEntity::Start();

		LeadBody = Physics::CreateBoxBody(this, Position, vec3(1.0f));

	}

	void Update()
	{
		mesh->Position = Position;
		mesh->Rotation = Rotation;
	}

	void LoadAssets() override
	{

		mesh = new StaticMesh(this);
		Drawables.push_back(mesh);

		mesh->LoadFromFile("GameData/models/cube.obj");
		mesh->ColorTexture = AssetRegistry::GetTextureFromFile("GameData/cat.png");

	}

	void NetSerialize(NetPacket& packet) override
	{

		if (LeadBody == nullptr) return;

		vec3 pos = Position;
		vec3 rot = Rotation;

		vec3 velocity = FromPhysics(LeadBody->GetLinearVelocity());
		vec3 angularVelocity = FromPhysics(LeadBody->GetAngularVelocity());

		packet.WriteVector3(pos);
		packet.WriteVector3(rot);
		packet.WriteVector3(velocity);
		packet.WriteVector3(angularVelocity);
	}

	void NetDeserialize(NetPacket& packet) override
	{

		if (LeadBody == nullptr) return;

		vec3 pos = packet.ReadVector3();
		vec3 rot = packet.ReadVector3();

		vec3 velocity = packet.ReadVector3();
		vec3 angularVelocity = packet.ReadVector3();

		Physics::SetBodyPositionAndRotation(LeadBody, pos, rot);
		Physics::SetLinearVelocity(LeadBody, velocity);
		Physics::SetAngularVelocity(LeadBody, angularVelocity);

	}

private:

};

REGISTER_ENTITY(NetworkTestCube, "networkTestCube")