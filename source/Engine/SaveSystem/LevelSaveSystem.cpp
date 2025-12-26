// LevelSaveSystem.cpp

#include "LevelSaveSystem.h"

#include <string>

#include "../Level.hpp"

#include "../Entity.h"

#include "../json.hpp"

#include "../LevelObjectFactory.h"

#include "../Physics.h"

#include "../FileSystem/FileSystem.h"

using namespace nlohmann;

LevelSaveData LevelSaveSystem::pendingSave;

LevelSaveData LevelSaveSystem::SaveLevelToData()
{

    LevelSaveData levelData;

    Level* level = Level::Current;

    levelData.GameTime = Time::GameTime;

    vector<EntitySaveData> entities;

    for (auto levelObject : level->LevelObjects)
    {

        Entity* entity = (Entity*)levelObject;

        if (entity == nullptr) continue;

        if (entity->SaveGame == false) continue;

        EntitySaveData data;
        data.className = entity->ClassName;
        data.name = entity->Name;
        data.id = entity->Id;
        
        json jsonData;

        entity->Serialize(jsonData);

        data.data = jsonData;

        entities.push_back(data);

    }

    levelData.deletedIDs = level->deletedIDs;
    levelData.deletedNames = level->deletedNames;

    levelData.name = level->filePath;
    levelData.entities = entities;
    levelData.nextId = level->nextId;

    return levelData;
}

void LevelSaveSystem::LoadLevelFromData(LevelSaveData data)
{

    Physics::ResetSimulation();
    Physics::SimulateWorldNoContacts();

    Time::DeltaTime = 0.0001;
    Time::DeltaTimeF = 0.0001f;

    Time::GameTimeNoPause = Time::GameTime = data.GameTime;

    for (auto id : data.deletedIDs)
    {
        Entity* target = Level::Current->FindEntityWithId(id);

        if (target == nullptr) continue;

        if (target->SaveGame == false) continue;

        target->Destroy();
    }


    for (auto name : data.deletedNames)
    {
        Entity* target = Level::Current->FindEntityWithName(name);

        if (target == nullptr) continue;

        if (target->SaveGame == false) continue;

        target->Destroy();
    }

    vector<Entity*> createdEntities;
    unordered_map<Entity*, EntitySaveData> pendingLoadEntities;

    for (auto entitySaveData : data.entities)
    {
        Entity* targetEntity = Level::Current->FindEntityWithId(entitySaveData.id);

        if (targetEntity == nullptr)
        {
            targetEntity = LevelObjectFactory::instance().create(entitySaveData.className);
            targetEntity->Name = entitySaveData.name;
            Level::Current->AddEntity(targetEntity);
            createdEntities.push_back(targetEntity);
        }
        pendingLoadEntities[targetEntity] = entitySaveData;

    }

    Level::Current->AddPendingLevelObjects();

    Physics::SimulateWorldNoContacts();

    for (auto entity : createdEntities)
    {
        entity->Start();
        entity->LoadAssetsIfNeeded();
    }

    Physics::SimulateWorldNoContacts();

    Level::Current->AddPendingLevelObjects();

    for (auto entity : pendingLoadEntities)
    {

        entity.first->Deserialize(entity.second.data);

    }

    Physics::SimulateWorldNoContacts();
    Physics::ResetSimulation();
    Logger::Log("save loaded\n");
}



void LevelSaveSystem::SaveLevelToFile(const string& saveName)
{
    auto levelSaveData = SaveLevelToData();

    json jsonData = json(levelSaveData);

    FileSystemEngine::WriteFile(saveDataPath + saveName + ".sav", jsonData.dump());
}

void LevelSaveSystem::LoadLevelFromFile(const string& saveName)
{

    string text = FileSystemEngine::ReadFile(saveDataPath + saveName + ".sav");;
    
    if (text.size() < 1)
    {
        Logger::Log("failed to load save" + saveName);
        return;
    }


    json jsonData = json::parse(text);

    LevelSaveData levelData = jsonData.get<LevelSaveData>();

    pendingSave = levelData;
    Level::LoadLevelFromFile(pendingSave.name);

}

void LevelSaveSystem::InitPersistence()
{
}
