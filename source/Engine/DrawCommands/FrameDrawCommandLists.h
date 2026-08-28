#pragma once

#include <vector>
#include "IDrawCommand.h"

// Built once per frame by Level::FinalizeFrame(), consumed by Renderer - the renderer only ever sees these three lists of IDrawCommand*, never an IDrawMesh/Entity/StaticMesh.
struct FrameDrawCommandLists
{
	// Every camera-visible command, unconditionally - drives the custom-id pass and DrawDetailShadows' candidate gathering.
	std::vector<IDrawCommand*> All;

	// Opaque + Masked, sorted by GetSortKey() then front-to-back.
	std::vector<IDrawCommand*> Depth;

	// Sorted strictly back-to-front, no batching.
	std::vector<IDrawCommand*> Transparent;

	void Clear()
	{
		All.clear();
		Depth.clear();
		Transparent.clear();
	}
};
