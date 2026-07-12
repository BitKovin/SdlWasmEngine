#include "MaterialFromTexture.h"

#include <algorithm>
#include <cctype>

#include <Helpers/StringHelper.h>

namespace
{
    bool ContainsCaseInsensitive(const std::string& haystack, const char* needle)
    {
        std::string lower = haystack;
        std::transform(lower.begin(), lower.end(), lower.begin(),
            [](unsigned char c) { return (char)std::tolower(c); });
        return lower.find(needle) != std::string::npos;
    }
}

// TODO(owner): replace this with the real regex-based texture/material
// lookup. This placeholder only does simple substring matching, e.g. a
// texture name containing "grass" becomes SpatialMaterial::Grass, "metal"
// becomes SpatialMaterial::Metal, etc.
SpatialMaterial GetMaterialFromTexture(const std::string& textureName)
{
    struct Rule { const char* needle; SpatialMaterial material; };


    static const Rule rules[] = {
        // Non-solid / gameplay-only textures — excluded from the audio scene
        // entirely (SpatialSoundManager skips any primitive mapped to Air).
        { "trigger",  SpatialMaterial::Air },
        { "clip",     SpatialMaterial::Air },
        { "skip",     SpatialMaterial::Air },
        { "hint",     SpatialMaterial::Air },
        { "nodraw",   SpatialMaterial::Air },

        { "/env/",   SpatialMaterial::Air },
        { "/skyboxes/",   SpatialMaterial::Air },
        { "/skies/",   SpatialMaterial::Air },
        { "/sky/",   SpatialMaterial::Air },

        { "grass",    SpatialMaterial::Grass },
        { "metal",    SpatialMaterial::Metal },
        { "water",    SpatialMaterial::Water },
        { "glass",    SpatialMaterial::Glass },
        { "sand",     SpatialMaterial::Sand },
        { "snow",     SpatialMaterial::Snow },
        { "ice",      SpatialMaterial::Ice },
        { "dirt",     SpatialMaterial::Dirt },
        { "mud",      SpatialMaterial::Mud },
        { "gravel",   SpatialMaterial::Gravel },
        { "marble",   SpatialMaterial::Marble },
        { "rock",     SpatialMaterial::Rock },
        { "stone",    SpatialMaterial::Rock },
        { "tile",     SpatialMaterial::Tile },
        { "brick",    SpatialMaterial::Brick },
        { "cloth",    SpatialMaterial::Cloth },
        { "carpet",   SpatialMaterial::Cloth },
        { "leaves",   SpatialMaterial::Leaf },
        { "leaf",     SpatialMaterial::Leaf },
        { "tree",     SpatialMaterial::Tree },
        { "gyprock",  SpatialMaterial::Gyprock },
        { "drywall",  SpatialMaterial::Gyprock },
        { "wood",     SpatialMaterial::WoodIndoor },
        { "concrete", SpatialMaterial::ConcretePolished },
    };

    for (const auto& rule : rules)
        if (ContainsCaseInsensitive(textureName, rule.needle))
            return rule.material;

    return SpatialMaterial::Concrete; // generic default for undefined level architecture
}
