#pragma once


class SpatialSoundManager
{
public:
	
	//called when the world is loaded or reloaded, builds the spatial sound world
	static void BuildWorld();

	//called every frame, updates the spatial sound world
	static void Update();
	


private:


};

