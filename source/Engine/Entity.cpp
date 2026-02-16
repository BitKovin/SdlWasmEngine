#include "Entity.h"
#include "LevelObjectFactory.h"

Entity* Entity::Spawn(std::string technicalName)
{
	auto entity = LevelObjectFactory::instance().create(technicalName);

	if (entity)
	{
		Level::Current->AddEntity(entity);
	}


	return entity;
}

void Entity::PreloadEntityType(std::string technicalName)
{

	if (Level::Current->IsEntityTypeLoaded(technicalName)) return; //already preloaded

	auto entity = Spawn(technicalName);

	if (entity)
	{
		//entity->Start();
		entity->LoadAssets();
		entity->SaveGame = false;
		entity->Destroy();
	}
	Level::Current->AddLoadedEntityType(technicalName);
}

void Entity::FinalLevelRemove()
{
	DestroyDrawables();
	//Logger::Log("FinalLevelRemove: " + Id);
}

void Entity::Destroy()
{

	if (Destroyed)
	{
		return;
	}

	Destroyed = true;

	if (OwnerId != "")
	{
		Entity* ownerRef = Level::Current->FindEntityWithId(OwnerId);
		if (ownerRef)
		{
			ownerRef->OnAction("despawned");
		}
	}

	Physics::DestroyBody(LeadBody);
	LeadBody = nullptr;
	for (Body* body : Bodies)
	{
		Physics::DestroyBody(body);
	}
	Bodies.clear();

	Level::Current->RemoveEntity(this);
}

void Entity::Serialize(json& target)
{
	SERIALIZE_FIELD(target, Name);
	SERIALIZE_FIELD(target, Position);
	SERIALIZE_FIELD(target, Rotation);
	SERIALIZE_FIELD(target, Scale);
	SERIALIZE_FIELD(target, Health);
	SERIALIZE_FIELD(target, destroyDelay);
	SERIALIZE_FIELD(target, Tags);
	SERIALIZE_FIELD(target, OwnerId);

}

LightVolPointData Entity::GetLightVolData(bool wallCheck)
{

	auto light = Level::Current->BspData.GetLightvolColorPoint(Position * MAP_SCALE, wallCheck);

	return light;
}

void Entity::Teleport(vec3 position)
{

	Position = position;

	if (LeadBody)
		Physics::SetBodyPosition(LeadBody, position);

}

void Entity::Deserialize(json& source)
{
	DESERIALIZE_FIELD(source, Name);
	DESERIALIZE_FIELD(source, Position);
	DESERIALIZE_FIELD(source, Rotation);
	DESERIALIZE_FIELD(source, Scale);
	DESERIALIZE_FIELD(source, Health);
	DESERIALIZE_FIELD(source, destroyDelay);
	DESERIALIZE_FIELD(source, Tags);
	DESERIALIZE_FIELD(source, OwnerId);
}