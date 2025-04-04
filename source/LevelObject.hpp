#pragma once

#include "EObject.hpp"

#include "IDrawMesh.h"

#include <vector>

using namespace std;

class LevelObject : public EObject
{
public:
	LevelObject(){}
	~LevelObject(){}

	virtual void Update() {}
	virtual void UpdatePhysics() {}

	virtual void Start() {}

	virtual void Finalize()
	{

	}

	virtual vector<IDrawMesh*> GetDrawMeshes() { return vector<IDrawMesh*>(); }


private:

};