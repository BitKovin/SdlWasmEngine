#pragma once

#include <string>

#include "SpatialMaterial.h"

// Placeholder texture -> material mapping, used when building the spatial
// audio world from BSP geometry (see SpatialSoundManager.cpp). Returns the
// engine-owned SpatialMaterial rather than vaudio's VAMaterialType, so this
// file — like everything else outside SpatialSoundManager.cpp — never has to
// include vaudio.h.
//
// This is a deliberately simple substring-matching placeholder, kept in its
// own file so it can be swapped out (e.g. for a real regex-based lookup)
// independently of SpatialSoundManager.
SpatialMaterial GetMaterialFromTexture(const std::string& textureName);
