#pragma once
#include <vector>

#include "LevelObject.hpp"
#include "EObject.hpp"

#include "IDrawMesh.h"

#include "mutex"

#include "Navigation/Navigation.hpp"

using namespace std;

class Level : EObject
{

private:
	vector<LevelObject*> LevelObjects = vector<LevelObject*>();

	mutex entityArrayLock = mutex();

public:

	

	static Level* Current;

	vector<IDrawMesh*> VissibleRenderList = vector<IDrawMesh*>();

	Level()
	{

	}

	~Level()
	{

	}

	static Level* OpenLevel()
	{

		if (Current)
		{
			Current->Dispose();
			free(Current);
		}

		Level* newLevel = new Level();

		Current = newLevel;

		NavigationSystem::GenerateNavData();

		return newLevel;

	}

	
	void AddEntity(LevelObject* entity)
	{
		entityArrayLock.lock();
		LevelObjects.push_back(entity);
		entityArrayLock.unlock();
	}

	void UpdatePhysics()
	{
		entityArrayLock.lock();
		for (auto var : LevelObjects)
		{
			var->UpdatePhysics();
		}
		entityArrayLock.unlock();
	}

	void Update()
	{
		entityArrayLock.lock();
		for (auto var : LevelObjects)
		{
			var->Update();
		}
		entityArrayLock.unlock();
	}

	void FinalizeFrame()
	{

		VissibleRenderList.clear();

		vector<IDrawMesh*> opaque;
		vector<IDrawMesh*> transparent;

		entityArrayLock.lock();
		for (auto var : LevelObjects)
		{
			var->Finalize();

			for (IDrawMesh* mesh : var->GetDrawMeshes())
			{
				if (mesh->IsCameraVisible())
				{
					if (mesh->Transparent)
					{
						transparent.push_back(mesh);
					}
					else
					{
						opaque.push_back(mesh);
					}
				}
			}

		}

		entityArrayLock.unlock();


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

	}

protected:

	void OnDispose()
	{
		for (auto var : LevelObjects)
		{
			var->Dispose();
		}
	}

};