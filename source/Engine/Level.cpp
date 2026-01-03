#include "Level.hpp"

#include "MapData.h"
#include "MapParser.h"

#include "Entity.h"

#include "Physics.h"

#include "EngineMain.h"

#include "LightSystem/LightManager.h"

#include "SaveSystem/LevelSaveSystem.h"

#include "LoadingScreen/LoadingScreenSystem.h"

#include "SoundSystem/SoundManager.hpp"

#include "AiPerception/AiPerceptionSystem.h"

#include "UI/RmlUi/RmlUiContext.h"

#include "LevelTraversalSystem.h"
#include "SaveSystem/GameSaveSystem.h"

Level* Level::Current = nullptr;

string Level::pendingLoadLevelPath = "";

void Level::CloseLevel()
{

	EngineMain::MainInstance->MainThreadPool->Stop();
	delete(EngineMain::MainInstance->MainThreadPool);

	Current->AddPendingLevelObjects();

	for (LevelObject* obj : Current->LevelObjects)
	{
		obj->Dispose();
	}
	

	Current->RemovePendingEntities();
	Current->MemoryCleanPendingEntities();

	Physics::DestroyAllBodies();

	Current->RemovePendingEntities();
	Current->MemoryCleanPendingEntities();

	NavigationSystem::DestroyAllObstacles();

	AiPerceptionSystem::RemoveAll();

	//cleaning all history of contacts because we delete all objects outside of game loop. Done to avoid crash
	MyContactListener::beforeSimulation();
	MyContactListener::afterSimulation();
	MyContactListener::beforeSimulation();
	MyContactListener::afterSimulation();

	EngineMain::MainInstance->RmlContext->UnloadAllDocuments();

	if (GameSaveSystem::LoadingGame == false && LevelTraversalSystem::Traveling == false)
	{
		LevelTraversalSystem::Reset();
	}

}

inline bool endsWith(const std::string& str, const std::string& suffix) {
	if (suffix.size() > str.size()) return false;
	return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

///UNSAFE. Loads level as soon as gets called
Level* Level::OpenLevel(string filePath)
{

	ChangingLevel = true;

	CurrentLevelChangeId++;

	bool isNewLevel = true;

	EngineMain::Viewport.ClearChildren();

	LoadingScreenSystem::Update(0);

	if (Current)
	{

		if (Current->filePath == filePath)
			isNewLevel = false;

		CloseLevel();

		Current->Dispose();
		delete(Current);
	}

	if (isNewLevel)
	{
		AssetRegistry::BeginLevelLoad();
		DebugDraw::ClearCommands();
		SoundManager::CleanAllData();
	}



	Time::Update();
	Time::DeltaTime = 0.0;
	Time::DeltaTimeF = 0;
	Time::GameTime = 0;
	Time::GameTimeNoPause = 0;

	Level* newLevel = new Level();

	newLevel->filePath = filePath;
	std::filesystem::path p(newLevel->filePath);
	newLevel->mapName = p.stem().string();

	Current = newLevel;


	EngineMain::MainInstance->MainThreadPool = new ThreadPool();

	EngineMain::MainInstance->MainThreadPool->Start(ThreadPool::GetNumThreadsForAsyncUpdate());

	if (endsWith(filePath, ".bsp"))
	{
		Current->BspData.LoadBSP(filePath.c_str());

		LoadingScreenSystem::Update(0.1f);

		Current->BspData.BuildVBO();

		Current->BspData.GenerateTexture();

		Current->BspData.GenerateLightmap();


		LoadingScreenSystem::Update(0.15f);

		Current->BspData.LoadToLevel();

		LoadingScreenSystem::Update(0.2f);

		Current->BspData.PreloadFaces();

		LoadingScreenSystem::Update(0.3f);

		Current->BspData.BuildMergedModels();

		LoadingScreenSystem::Update(0.4f);

		Current->BspData.BuildStaticOpaqueObstacles();
	}
	else
	{
		MapData mapData = MapParser::ParseMap(filePath);

		mapData.LoadToLevel();
	}

	LoadingScreenSystem::Update(0.5f);

	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Master.bank");
	SoundManager::LoadBankFromPath("GameData/sounds/banks/Desktop/Master.strings.bank");

	Current->AddPendingLevelObjects();
	Current->RemovePendingEntities();
	Current->MemoryCleanPendingEntities();

	Current->LoadAssets();

	for (LevelObject* obj : Current->LevelObjects)
	{
		obj->Start();
	}

	auto gamestart = LevelObjectFactory::instance().create("gamestart");

	if (gamestart)
	{
		Current->LevelObjects.insert(Current->LevelObjects.begin(), gamestart);
	}
	

	Current->AddPendingLevelObjects();
	Current->RemovePendingEntities();
	Current->MemoryCleanPendingEntities();

	LoadingScreenSystem::Update(0.6f);

	if (isNewLevel) 
	{
		printf("generating nav mesh\n");

		NavigationSystem::GenerateNavData();

		printf("generated nav mesh\n");
	}

	LoadingScreenSystem::Update(0.9f);

	Time::Update();
	Time::DeltaTime = 0.0;
	Time::DeltaTimeF = 0;
	Time::GameTime = 0;
	Time::GameTimeNoPause = 0;

	if (LevelSaveSystem::pendingSave.name == Current->filePath)
	{
		LevelSaveSystem::LoadLevelFromData(LevelSaveSystem::pendingSave);

		LevelSaveSystem::pendingSave = LevelSaveData();

	}


	LevelTraversalSystem::FinishTransition();

	Current->LoadAssets();

	Current->AddPendingLevelObjects();
	Current->RemovePendingEntities();
	Current->MemoryCleanPendingEntities();


	Time::Update();
	Time::Update();

	LoadingScreenSystem::Update(1);

	Physics::ResetSimulation();

	ChangingLevel = false;

	return newLevel;
}

void Level::AddLoadedEntityType(const std::string& className)
{

	std::lock_guard<std::recursive_mutex> lock(loadedEntitiesLock);

	loadedEntityTypes.emplace(className);

}

bool Level::IsEntityTypeLoaded(const std::string& className)
{

	std::lock_guard<std::recursive_mutex> lock(loadedEntitiesLock);

	return loadedEntityTypes.find(className) != loadedEntityTypes.end();

}

void Level::AddEntity(LevelObject* obj)
{

	std::lock_guard<std::recursive_mutex> lock(pendingEntityArrayLock);

	Entity* entity = dynamic_cast<Entity*>(obj);

	if (entity)
	{
		string classname = entity->ClassName;

		auto nId = nextId.find(classname);

		if (nId == nextId.end())
		{
			nextId[classname] = 0;
		}

		int id = nextId[classname];

		nextId[classname]++;

		string entId = "$" + classname + "_" + to_string(id);

		entity->Id = entId;

		std::unique_lock<std::shared_mutex> lock(entityNameMapMutex);

		entityIdMap[entId] = entity;
		entityNameMap[entity->Name] = entity;

	}

	pendingAddLevelObjects.push_back(obj);

	
}

void Level::RemoveEntity(LevelObject* obj)
{
	std::lock_guard<std::recursive_mutex> lock(pendingEntityArrayLock);

	Entity* entity = (Entity*)obj;

	if (entity)
	{
		if (entity->SaveGame)
		{
			if (entity->Unique && entity->Name != "")
			{
				deletedNames.push_back(entity->Name);
			}
			else
			{
				deletedIDs.push_back(entity->Id);
			}
		}
		std::unique_lock<std::shared_mutex> lock(entityNameMapMutex);

		entityIdMap.erase(entity->Id);
		entityNameMap.erase(entity->Name);

	}

	PendingRemoveLevelObjects.push_back(obj);
}

void Level::AsyncUpdate(bool paused)
{
	AddPendingLevelObjects();
	RemovePendingEntities();

	entityArrayLock.lock();

	auto objects = LevelObjects;

	entityArrayLock.unlock();

	std::vector<std::function<void()>> updateJobs;
	updateJobs.reserve(objects.size()); // Pre-allocate for efficiency

	for (auto var : objects) {
		if (var->UpdateWhenPaused || paused == false) {
			updateJobs.emplace_back([var]() { var->AsyncUpdate(); });
		}
	}

	if (!updateJobs.empty()) {
		asyncUpdateThreadPool->QueueJobs(std::move(updateJobs));
	}

	asyncUpdateThreadPool->WaitForFinish();
	AddPendingLevelObjects();
	RemovePendingEntities();
}

void Level::RemovePendingEntities()
{

	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
	std::lock_guard<std::recursive_mutex> lockP(pendingEntityArrayLock);

	for (auto& entity : PendingRemoveLevelObjects)
	{

		if (entity == nullptr) continue;

		auto it = std::find(LevelObjects.begin(), LevelObjects.end(), entity);
		if (it != LevelObjects.end())
		{
			LevelObjects.erase(it);
		}

		PendingMemoryCleanObjects.push_back(entity);

		entity = nullptr;

	}

	PendingRemoveLevelObjects.clear();

}

void Level::MemoryCleanPendingEntities()
{

	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
	std::lock_guard<std::recursive_mutex> lockP(pendingEntityArrayLock);

	DeletedLevelObjectAdresses = std::unordered_set<void*>();
	DeletedLevelObjectAdresses.reserve(PendingMemoryCleanObjects.size());

	for (auto& entity : PendingMemoryCleanObjects)
	{

		if (entity == nullptr) continue;

		if (DeletedLevelObjectAdresses.find(entity) != DeletedLevelObjectAdresses.end())
		{
			Logger::Log("entity was deleted twice. possible crash avoided");
			continue;
		}

		DeletedLevelObjectAdresses.insert(entity);

		entity->FinalLevelRemove();
		delete(entity);

	}

	PendingMemoryCleanObjects.clear();
	PendingMemoryCleanObjects = vector<LevelObject*>();

}

void Level::Update(bool paused)
{
	AddPendingLevelObjects();
	RemovePendingEntities();

	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
	for (auto var : LevelObjects)
	{
		if (var->UpdateWhenPaused || paused == false && var->Destroyed == false)
		{
			var->Update();
		}

	}

	AddPendingLevelObjects();
	RemovePendingEntities();
}

void Level::LateUpdate(bool paused)
{
	AddPendingLevelObjects();
	RemovePendingEntities();

	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
	for (auto var : LevelObjects)
	{
		if (var->LateUpdateWhenPaused || paused == false && var->Destroyed == false)
		{
			var->LateUpdate();
		}
	}
	AddPendingLevelObjects();
	RemovePendingEntities();
}

vector<Entity*> Level::FindAllEntitiesWithName(const hashed_string& name)
{

	vector<Entity*> result;

	entityArrayLock.lock();
	pendingEntityArrayLock.lock();

	auto curLevelObjects = LevelObjects;
	auto pendingLevelObjects = pendingAddLevelObjects;

	entityArrayLock.unlock();
	pendingEntityArrayLock.unlock();

	for (auto var : curLevelObjects)
	{
		Entity* entity = (Entity*)var;

		if (entity && entity->Name == name.str() && entity->Destroyed == false)
		{
			result.push_back(entity);
		}

	}

	for (auto var : pendingLevelObjects)
	{
		Entity* entity = (Entity*)var;

		if (entity && entity->Name == name.str() && entity->Destroyed == false)
		{
			result.push_back(entity);
		}

	}

	return result;
}

Entity* Level::FindEntityWithName(const hashed_string& name)
{

	std::shared_lock<std::shared_mutex> lock(entityNameMapMutex); 

	auto res = entityNameMap.find(name);

	if (res != entityNameMap.end())
	{
		return res->second;
	}
	
	return nullptr;

}

Entity* Level::FindEntityWithId(const hashed_string& id)
{

	std::shared_lock<std::shared_mutex> lock(entityNameMapMutex);  // shared/read lock

	auto res = entityIdMap.find(id);

	if (res != entityIdMap.end())
	{
		return res->second;
	}	

	return nullptr;

}

void Level::FinalizeFrame()
{
	AddPendingLevelObjects();

	VissibleRenderList.clear();

	vector<IDrawMesh*> opaque;
	vector<IDrawMesh*> transparent;

	vector<IDrawMesh*> ShadowCasters;
	vector<IDrawMesh*> DetailShadowCasters;

	{

		bool renderAll = EngineMain::MainInstance->LoadingFrames > 1;

		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

		for (auto var : LevelObjects)
		{

			var->Finalize();

			for (IDrawMesh* mesh : var->GetDrawMeshes())
			{		

				if (mesh->IsCameraVisible() || renderAll)
				{			

					if (mesh->Transparent)
					{
						transparent.push_back(mesh);
					}
					else
					{
						opaque.push_back(mesh);
					}

					if (mesh->WasRended == false)
					{
						mesh->WasRended = true;

						mesh->StartedRendering();
					}

					mesh->LastRenderedTime = Time::GameTime;
					mesh->WasRended = true;

					mesh->FinalizeFrameData();

				}
				else
				{
					mesh->WasRended = false;
				}

				if (mesh->IsShadowVisible() || renderAll)
				{

					if (mesh->IsDetailShadow())
					{
						DetailShadowCasters.push_back(mesh);
					}
					else
					{
						ShadowCasters.push_back(mesh);
					}

					
				}
				

			}

		}
	}



	// Sort opaque objects from closest to farthest (ascending order by distance).
	std::sort(opaque.begin(), opaque.end(),
		[](IDrawMesh* a, IDrawMesh* b) {
			return a->GetDistanceToCamera() < b->GetDistanceToCamera();
		});

	// Sort transparent objects from farthest to closest (descending order by distance).
	std::sort(transparent.begin(), transparent.end(),
		[](IDrawMesh* a, IDrawMesh* b) {
			return a->GetDistanceToCamera() > b->GetDistanceToCamera();
		});

	// Append sorted opaque objects first.
	for (auto mesh : opaque)
	{
		VissibleRenderList.push_back(mesh);
	}

	// Append sorted transparent objects second.
	for (auto mesh : transparent)
	{
		VissibleRenderList.push_back(mesh);
	}

	ShadowRenderList = ShadowCasters;
	DetailShadowRenderList = DetailShadowCasters;

	LightManager::Update();

}
