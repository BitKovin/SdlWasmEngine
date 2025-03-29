#pragma once
#include <vector>

#include "LevelObject.hpp"
#include "EObject.hpp"

using namespace std;

class Level : EObject
{
public:

	vector<LevelObject*> LevelObjects = vector<LevelObject*>();

	static Level* Current;

	Level()
	{

	}

	~Level()
	{

	}

	static Level* OpenLevel()
	{

		Level* newLevel = new Level();

		Current = newLevel;


		return newLevel;

	}

	
	void AddEntity(LevelObject* entity)
	{
		LevelObjects.push_back(entity);
	}

	void Update()
	{
		for (auto var : LevelObjects)
		{
			var->Update();
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