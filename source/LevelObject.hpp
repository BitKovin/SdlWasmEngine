#pragma once

#include "EObject.hpp"

class Level;

class LevelObject : public EObject
{
public:
	LevelObject(){}
	~LevelObject(){}

	virtual void Update() {}

	virtual void Start() {}



private:

};