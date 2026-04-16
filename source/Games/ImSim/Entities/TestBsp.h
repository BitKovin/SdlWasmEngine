#pragma once
#include <Entity.h>
#include <BSP/Quake3Bsp.h>

#include <MapParser.h>

class TestBsp : public Entity
{
public:
	TestBsp();
	~TestBsp();

private:

	CQuake3BSP* bsp;

};



TestBsp::TestBsp()
{
	bsp = new CQuake3BSP();
	bsp->LoadBSP("GameData/maps/test.bsp");
	bsp->BuildVBO();
	bsp->GenerateTexture();
	bsp->GenerateLightmap();

	auto entities = MapParser::ParseBSPEntities(bsp->entities);
	

	Drawables.push_back(bsp);
}

TestBsp::~TestBsp()
{
}