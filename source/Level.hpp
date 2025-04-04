#pragma once
#include <vector>

#include "LevelObject.hpp"
#include "EObject.hpp"

#include "IDrawMesh.h"

#include "mutex"

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

		entityArrayLock.lock();
		for (auto var : LevelObjects)
		{
			var->Finalize();

			for (IDrawMesh* mesh : var->GetDrawMeshes())
			{
				if (mesh->IsCameraVisible())
				{
					VissibleRenderList.push_back(mesh);
				}
			}

		}

		entityArrayLock.unlock();

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