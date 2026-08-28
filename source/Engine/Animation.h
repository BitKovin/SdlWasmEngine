#pragma once
#include "SkeletalMesh.hpp"

class Animation : public SkeletalMesh
{
public:

	Animation(Entity* owner) : SkeletalMesh(owner)
	{
		skipMeshLoad = true;
		Visible = false;
	}

	bool IsInFrustrum(Frustum frustrum)	{return false;};

	virtual bool isVisible() override{return false;}

	bool IsCameraVisible(){return false;}

	bool IsShadowVisible(){return false;}

	void CollectDrawCommands(vector<IDrawCommand*>& outCommands) override {}

private:

};
