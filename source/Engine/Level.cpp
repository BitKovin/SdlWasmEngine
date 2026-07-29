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

#include "SpatialSound/SpatialSoundManager.h"

#include <World/WorldOrientationManager.h>
#include <Logger.hpp>

#include <tracy/tracy/Tracy.hpp>

#include <Network/NetworkManager.h>
#include <Network/NetworkedEntity.h>
#include <IDrawMesh.h>

Level* Level::Current = nullptr;

string Level::pendingLoadLevelPath = "";

bool Level::HasPendingLevelLoad()
{
	return pendingLoadLevelPath.empty() == false;
}

void Level::CloseLevel()
{

	EngineMain::MainInstance->MainThreadPool->Stop();
	delete(EngineMain::MainInstance->MainThreadPool);
	EngineMain::MainInstance->MainThreadPool = nullptr;

	Current->AddPendingLevelObjects();
	for (LevelObject* obj : Current->LevelObjects)
	{
		obj->Dispose();
		Current->RemoveEntity(obj);
	}
	

	NetworkManager::Tick(0.5f);


	Current->AddPendingLevelObjects();
	for (LevelObject* obj : Current->LevelObjects)
	{
		obj->Dispose();
		Current->RemoveEntity(obj);
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

	Input::ReleaseAllActions();

	if (GameSaveSystem::LoadingGame == false && LevelTraversalSystem::Traveling == false)
	{
		LevelTraversalSystem::Reset();
	}

	WorldOrientationManager::Reset();

}

inline bool endsWith(const std::string& str, const std::string& suffix) {
	if (suffix.size() > str.size()) return false;
	return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

///UNSAFE. Loads level as soon as gets called
Level* Level::OpenLevel(string filePath)
{

	EngineMain::MainInstance->Paused = false;

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

	//AssetRegistry::ClearMemory();

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

	NetworkManager::BeginLevelLoad(Current);

	EngineMain::MainInstance->MainThreadPool = new ThreadPool("Tread Pool");

	EngineMain::MainInstance->MainThreadPool->Start(ThreadPool::GetNumThreadsForThreadPool());

	if (endsWith(filePath, ".bsp"))
	{
		Current->BspData.LoadBSP(filePath.c_str());

		LoadingScreenSystem::Update(0.1f);

		Current->BspData.BuildVBO();
		Current->BspData.GenerateTexture();
		Current->BspData.GenerateLightmap();

		std::string portalFile = FileSystemEngine::ReadFile(StringHelper::Replace(filePath, ".bsp", ".prt"));
		if(portalFile.empty() == false) Current->BspData.LoadPortalsFromPRT(portalFile);

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

	LoadingScreenSystem::Update(0.6f);

	Current->AddPendingLevelObjects();

	float assetPreloadProgress = 0;

	for (auto obj : Current->LevelObjects)
	{

		assetPreloadProgress += 1.0 / Current->LevelObjects.size();

		LoadingScreenSystem::Update(0.6 + assetPreloadProgress * 0.3);

		obj->LoadAssetsIfNeeded();
	}

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

	LoadingScreenSystem::Update(0.9f);

	if (isNewLevel) 
	{
		Logger::Info("generating nav mesh");

		NavigationSystem::GenerateNavData();

		Logger::Info("generated nav mesh");
	}

	SpatialSoundManager::BuildWorld();

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

	NetworkManager::OnLevelLoaded();

	//time for client to get server packets
	if (NetworkManager::IsServer() == false)
	{
		NetworkManager::Tick(0.5);
		this_thread::sleep_for(300ms);
		NetworkManager::Tick(0.5);
		this_thread::sleep_for(300ms);
		NetworkManager::Tick(0.5);
	}

	for (auto o : Level::Current->LevelObjects)
	{
		o->PostLoadStart();
	}

	if (pendingLoadLevelPath != Current->filePath)//if we connected to server that has another level
	{
		return OpenLevel(pendingLoadLevelPath);
	}

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

void Level::AddEntity(LevelObject* obj, bool imOwner)
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

	if (!NetworkManager::IsActive()) return;

	auto* ne = dynamic_cast<NetworkedEntity*>(entity);
	if (!ne) return; 

	if (NetworkManager::IsLoadingLevel()) {
		// Load phase: silent registration, identical on all peers
		uint32_t netId = NetworkManager::MakeLoadPhaseId(entity->Id);
		assert(NetworkManager::Find(netId) == nullptr &&
			"Load-phase networkId collision — rename one of these entities");
		ne->networkId = netId;
		ne->networkOwner = 0;
		ne->isOwned = NetworkManager::IsServer();
		NetworkManager::Register(ne);
		// OnNetworkSpawn() fires later in NetworkManager::OnLevelReady()

	}
	else if (ne->networkId == 0) {
		// Runtime: fresh local spawn
		uint8_t owner = imOwner ? NetworkManager::GetLocalPeerId() : 0;
		ne->networkOwner = owner;
		ne->isOwned = true;
		ne->networkId = NetworkManager::AllocateRuntimeId(owner);
		NetworkManager::Register(ne);
		NetworkManager::BroadcastSpawn(ne);
		ne->OnNetworkSpawn();

	}
	else {
		// Runtime: received from network — networkId/networkOwner already set
		ne->isOwned = (ne->networkOwner == NetworkManager::GetLocalPeerId());
		NetworkManager::Register(ne);
		ne->OnNetworkSpawn();
	}
	
}


void Level::RemoveEntitySilent(LevelObject* obj)
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
		entity->DestroyPhysics();
	}

	PendingRemoveLevelObjects.push_back(obj);
}

void Level::RemoveEntity(LevelObject* obj)
{
	std::lock_guard<std::recursive_mutex> lock(pendingEntityArrayLock);

	RemoveEntitySilent(obj);


	if (NetworkManager::IsActive()) {
		if (auto* ne = dynamic_cast<NetworkedEntity*>(obj)) {
			ne->OnNetworkDespawn();
			if (ne->isOwned || NetworkManager::IsServer()) {
				NetworkManager::BroadcastDespawn(ne->networkId);
			}
			NetworkManager::Unregister(ne->networkId);
		}
	}

}

void Level::AsyncUpdate(bool paused)
{
	AddPendingLevelObjects();
	RemovePendingEntities();

	entityArrayLock.lock();
	auto objects = LevelObjects;
	entityArrayLock.unlock();

	std::vector<LevelObject*> toUpdate;
	toUpdate.reserve(objects.size());

	for (auto var : objects)
	{
		if ((var->UpdateWhenPaused || !paused) && var->UpdateEnabled && var->wantsAsyncUpdate)
			toUpdate.push_back(var);
	}

	if (!toUpdate.empty())
	{
		asyncUpdateThreadPool->ParallelFor(toUpdate.size(), [&toUpdate](size_t i)
			{
				ZoneScoped;
				std::string zoneName = "Async Update Entity" + toUpdate[i]->GetId();
				ZoneName(zoneName.c_str(), zoneName.size());
				toUpdate[i]->AsyncUpdate();
			});
	}

	AddPendingLevelObjects();
	RemovePendingEntities();
}

void Level::PreFinalize()
{
	AddPendingLevelObjects();
	RemovePendingEntities();

	entityArrayLock.lock();
	auto objects = LevelObjects;
	entityArrayLock.unlock();

	// Flatten object -> draw-mesh into one indexable list so ParallelFor
	// splits it into equal contiguous chunks, regardless of how unevenly
	// meshes are distributed across objects.
	struct DrawMeshWork {
		LevelObject* var;
		IDrawMesh* mesh;
	};

	std::vector<DrawMeshWork> work;
	work.reserve(objects.size());

	for (auto var : objects)
		for (auto d : var->GetDrawMeshes())
			work.push_back({ var, d });

	if (!work.empty())
	{
		asyncUpdateThreadPool->ParallelFor(work.size(), [&work](size_t i)
			{
				ZoneScoped;
				std::string zoneName = "draw object pre finalize" + work[i].var->GetId();
				ZoneName(zoneName.c_str(), zoneName.size());
				work[i].mesh->PreFinalize();
			});
	}

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
			//Logger::Log("entity was deleted twice. possible crash avoided");
			continue;
		}

		DeletedLevelObjectAdresses.insert(entity);

		entity->FinalLevelRemove();
		delete(entity);

	}

	PendingMemoryCleanObjects.clear();
	PendingMemoryCleanObjects = vector<LevelObject*>();

}

void Level::LoadAssetsIfNeeded()
{

	std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

	for (auto var : LevelObjects)
	{
		var->LoadAssetsIfNeeded();
	}

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
			if (var->UpdateEnabled)
			{
				ZoneScoped;
				std::string zoneName = "Update Entity" + var->GetId();
				ZoneName(zoneName.c_str(), zoneName.size());
				var->Update();
			}
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
			if (var->UpdateEnabled)
			{
				ZoneScoped;
				std::string zoneName = "LateUpdate Entity" + var->GetId();
				ZoneName(zoneName.c_str(), zoneName.size());
				var->LateUpdate();
			}
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

				if ((mesh->IsCameraVisible() && var->Visible) || renderAll)
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
