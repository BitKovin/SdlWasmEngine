#pragma once

#include "EObject.hpp"

#include "IDrawMesh.h"

#include <vector>

using namespace std;

class LevelObject : public EObject
{
public:

	bool Static = false;

	bool Destroyed = false;

	bool UpdateWhenPaused = false;
	bool LateUpdateWhenPaused = false;

	bool UpdateEnabled = true;

	bool wantsAsyncUpdate = true;

	bool Visible = true;

	LevelObject(){}
	virtual ~LevelObject() = default;

	virtual void Update() {}
	virtual void LateUpdate() {}
	virtual void UpdatePhysics() {}

	//never call base class implementation of this
	virtual void AsyncUpdate() { wantsAsyncUpdate = false; }

	virtual void Start() {}

	//called after level and save is loaded. If you want to spawn something on start and keep ownership - use this one
	virtual void PostLoadStart() {}

	virtual void UpdateDebugUI() {}

	virtual void FinalLevelRemove() {}

	virtual void Finalize()
	{

	}

	virtual std::string GetId()
	{
		return "";
	}

	virtual void LoadAssetsIfNeeded(){}

	virtual vector<IDrawMesh*> GetDrawMeshes() { return vector<IDrawMesh*>(); }


private:

};