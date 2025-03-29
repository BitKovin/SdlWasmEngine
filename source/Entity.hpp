#pragma once

#include <vector>

#include "EObject.hpp"
#include "LevelObject.hpp"

#include "glm.h"

using namespace std;

class Entity : public LevelObject
{
public:

	vec3 Position = vec3();

	vec3 Rotation = vec3();

	Entity()
	{

	}
	~Entity()
	{

	}

	void virtual Destroy()
	{

	}

private:

};