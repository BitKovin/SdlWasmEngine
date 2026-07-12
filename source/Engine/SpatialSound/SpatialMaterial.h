#pragma once

// Engine-owned mirror of vaudio's VAMaterialType, so that GetMaterialFromTexture()
// (and anything else that classifies surface materials) never has to include
// vaudio.h. SpatialSoundManager.cpp — the only file allowed to include
// vaudio.h — converts this to VAMaterialType internally.
//
// Keep this in sync with VAMaterialType in vaudio.h by hand; there's no
// compile-time check tying the two together, since that's exactly the point
// of keeping them separate.
enum class SpatialMaterial
{
    Air = 0,   // Non-solid — anything mapped to Air is excluded from the audio scene entirely.
    Brick,
    Cloth,
    Concrete,
    ConcretePolished,
    Dirt,
    Glass,
    Grass,
    Gravel,
    Gyprock,
    Ice,
    Leaf,
    Marble,
    Metal,
    Mud,
    Rock,
    Sand,
    Snow,
    Tile,
    Tree,
    Water,
    WoodIndoor,
    WoodOutdoor,
};
