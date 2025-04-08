#pragma once

#include "StaticMesh.hpp"

#include "glm.h"
#include "gl.h"

class DebugDrawCommand
{
public:
	DebugDrawCommand();
	~DebugDrawCommand();

	void Draw()
	{

	}

private:

};

DebugDrawCommand::DebugDrawCommand()
{
}

DebugDrawCommand::~DebugDrawCommand()
{
}

class DebugDraw
{
public:
	
	static vector<> Instance;

private:

};