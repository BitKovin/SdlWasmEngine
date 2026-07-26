#pragma once
#include <vector>
#include <unordered_set>
#include "LevelObject.hpp"
#include "EObject.hpp"

#include "IDrawMesh.h"

#include "mutex"
#include <shared_mutex>

#include "Navigation/Navigation.hpp"

#include "MeshUtils.hpp"

#include "ThreadPool.h"

#include "BSP/Quake3Bsp.h"

#include "utility/hashed_string.hpp"

#include "malloc_override.h"


class Entity;

class Level : EObject
{

private:
	
	vector<LevelObject*> PendingRemoveLevelObjects = vector<LevelObject*>();
	vector<LevelObject*> PendingMemoryCleanObjects = vector<LevelObject*>();


	std::recursive_mutex entityArrayLock = std::recursive_mutex();
	std::recursive_mutex pendingEntityArrayLock = std::recursive_mutex();

	std::recursive_mutex loadedEntitiesLock = std::recursive_mutex();

	static std::string pendingLoadLevelPath;

	std::vector<LevelObject*> pendingAddLevelObjects;

	std::unordered_set<std::string> loadedEntityTypes;
	
	std::unordered_map<hashed_string, Entity*> entityIdMap;
	std::unordered_map<hashed_string, Entity*> entityNameMap;

	std::shared_mutex entityNameMapMutex;


public:

	std::string Identifier = ""; //used for level traversal memory to keep in memory same file, but under different keys for save game. Keep empty to use file path instead.

	ThreadPool* asyncUpdateThreadPool;

	static Level* Current;

	vector<LevelObject*> LevelObjects = vector<LevelObject*>(); //not safe to use outside of class
	std::unordered_map<std::string, int> nextId;
	std::vector<std::string> deletedNames;
	std::vector<std::string> deletedIDs;

	vector<IDrawMesh*> VissibleRenderList = vector<IDrawMesh*>();
	vector<IDrawMesh*> ShadowRenderList = vector<IDrawMesh*>();
	vector<IDrawMesh*> DetailShadowRenderList = vector<IDrawMesh*>();


	string filePath;
	string mapName;

	CQuake3BSP BspData;

	std::unordered_set<void*> DeletedLevelObjectAdresses;

	inline static bool ChangingLevel = false;

	inline static uint16 CurrentLevelChangeId = 0;

	Level()
	{
		asyncUpdateThreadPool = new ThreadPool("Async Update Pool");
		asyncUpdateThreadPool->Start(ThreadPool::GetNumThreadsForAsyncUpdate());
	}

	~Level()
	{
		asyncUpdateThreadPool->Stop();
		delete(asyncUpdateThreadPool);
	}

	static bool HasPendingLevelLoad();

	static void CloseLevel();

	static Level* OpenLevel(string filePath);

	static void LoadLevelFromFile(string filePath)
	{
		pendingLoadLevelPath = filePath;
	}

	static bool LoadPendingLevel()
	{
		if (pendingLoadLevelPath != "")
		{
			OpenLevel(pendingLoadLevelPath);
			pendingLoadLevelPath = "";

			return true;
		}

		
		return false;
	}

	MeshUtils::PositionVerticesIndices GetStaticNavObstaclesMesh()
	{
		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

		 vector<MeshUtils::PositionVerticesIndices> meshes;

		for (auto obj : LevelObjects)
		{
			if(obj->Static)
			for (auto mesh : obj->GetDrawMeshes()) 
			{

				if (mesh->StaticNavigation == false) continue;

				auto obstacles = mesh->GetNavObstacleMeshes();


				meshes.insert(meshes.end(), obstacles.begin(), obstacles.end());

			}
		}



		MeshUtils::PositionVerticesIndices resultMesh = MeshUtils::MergeMeshes(meshes);

		return resultMesh;

	}

	void AddLoadedEntityType(const std::string& className);

	bool IsEntityTypeLoaded(const std::string& className);

	void LoadAssets()
	{
		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

		AddPendingLevelObjects();

		for (auto obj : LevelObjects)
		{
			obj->LoadAssetsIfNeeded();
		}
	}
	
	void AddEntity(LevelObject* obj, bool imOwner = true);

	void RemoveEntitySilent(LevelObject* obj);

	void RemoveEntity(LevelObject* obj);

	void AddPendingLevelObjects()
	{
		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
		std::lock_guard<std::recursive_mutex> lockP(pendingEntityArrayLock);
		for (auto entity : pendingAddLevelObjects)
		{
			LevelObjects.push_back(entity);
		}
		pendingAddLevelObjects.clear();
	}

	void RemovePendingEntities();

	void MemoryCleanPendingEntities();

	void UpdatePhysics()
	{
		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);

		for (auto var : LevelObjects)
		{
			var->UpdatePhysics();
		}
		AddPendingLevelObjects();
	}

	void LoadAssetsIfNeeded();

	void Update(bool paused);

	void LateUpdate(bool paused);

	void AsyncUpdate(bool paused);

	void DevUiUpdate()
	{
		std::lock_guard<std::recursive_mutex> lock(entityArrayLock);
		for (auto var : LevelObjects)
		{
			var->UpdateDebugUI();
		}

	}

	vector<Entity*> FindAllEntitiesWithName(const hashed_string& name);
	Entity* FindEntityWithName(const hashed_string& name);
	Entity* FindEntityWithId(const hashed_string& id);

	void FinalizeFrame();

protected:

	void OnDispose()
	{
		for (auto var : LevelObjects)
		{
			var->Dispose();
		}
	}

};