#pragma once

#include <string>
#include <vector>
#include <map>
#include "glm.h"

// BrushData: Represents brush texture and coordinate information.
class BrushData {
public:
    std::string Texture;
    float TextureScale = 1.0f;
    std::vector<float> TextureCoordinates;
    std::string Name;
};

// EntityData: Holds properties, brushes and provides helper getters.
class EntityData {
public:
    std::string Classname;
    std::map<std::string, std::string> Properties;
    std::vector<BrushData> Brushes;
    std::string name;

    glm::vec3 GetPropertyVectorPosition(const std::string& propName);
    glm::vec3 GetPropertyVectorRotation(const std::string& propName);
    glm::vec3 GetPropertyVector(const std::string& propName, const glm::vec3& def);
    float GetPropertyFloat(const std::string& propName, float defaultValue = 0.0f);
    bool GetPropertyBool(const std::string& propName, bool defaultValue = false);
    std::string GetPropertyString(const std::string& propName, const std::string& defaultValue = "");

    // ConvertRotation adjusts an imported rotation vector.
    static glm::vec3 ConvertRotation(glm::vec3 importRot, bool notForModel = false);
};

// MapData: Contains general map data, a list of entities and static values.
class MapData {
public:
    std::string Game;
    std::string Path;
    std::string Format;
    std::vector<EntityData> Entities;

    // Static members (configuration options)
    static bool MergeBrushes;
    static float UnitSize;
};

// Helper to retrieve an entity by classname.
EntityData* GetEntityDataFromClass(MapData& mapData, const std::string& className);