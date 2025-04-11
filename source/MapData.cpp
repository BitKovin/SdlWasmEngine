#include "MapData.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include "glm.h"

#include "Level.hpp"

#include "Entity.hpp"

#include "BrushFaceMesh.hpp"

#include "LevelObjectFactory.h"

// Static member definitions.
bool MapData::MergeBrushes = false;
float MapData::UnitSize = 32.0f;

// Helper function for splitting a string by whitespace.
static std::vector<std::string> split_whitespace(const std::string& s) {
    std::vector<std::string> tokens;
    std::istringstream iss(s);
    std::string token;
    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// Safe string to float conversion
static bool safe_stof(const std::string& s, float& value) {
    if (s.empty()) {
        return false;
    }
    char* endptr;
    value = std::strtof(s.c_str(), &endptr);
    // Conversion is successful only if the entire string was consumed
    return (endptr == s.c_str() + s.size());
}

// -------------------
// EntityData Methods
// -------------------

/**
     * Retrieves a 3D position vector from a property, scaling it by MapData::UnitSize.
     * Returns (0, 0, 0) if the property is invalid or missing.
     */
glm::vec3 EntityData::GetPropertyVectorPosition(const std::string& propName) {
    auto it = Properties.find(propName);
    if (it == Properties.end()) {
        return glm::vec3(0.0f);
    }

    std::string value = it->second;
    value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
    auto parts = split_whitespace(value);
    if (parts.size() < 3) {
        return glm::vec3(0.0f);
    }

    float x, y, z;
    if (!safe_stof(parts[0], x) || !safe_stof(parts[2], y) || !safe_stof(parts[1], z)) {
        return glm::vec3(0.0f);
    }

    z *= -1.0f; // Adjust z-coordinate
    return glm::vec3(x, y, z) / MapData::UnitSize;
}

/**
 * Retrieves a 3D rotation vector from a property.
 * Returns (0, 0, 0) if the property is invalid or missing.
 */
glm::vec3 EntityData::GetPropertyVectorRotation(const std::string& propName) {
    auto it = Properties.find(propName);
    if (it == Properties.end()) {
        return glm::vec3(0.0f);
    }

    std::string value = it->second;
    value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
    auto parts = split_whitespace(value);
    if (parts.size() < 3) {
        return glm::vec3(0.0f);
    }

    float x, y, z;
    if (!safe_stof(parts[0], x) || !safe_stof(parts[1], y) || !safe_stof(parts[2], z)) {
        return glm::vec3(0.0f);
    }

    return glm::vec3(x, y, z);
}

/**
 * Retrieves a 3D vector from a property with a custom default value.
 * Returns the default value if the property is invalid or missing.
 */
glm::vec3 EntityData::GetPropertyVector(const std::string& propName, const glm::vec3& def) {
    auto it = Properties.find(propName);
    if (it == Properties.end()) {
        return def;
    }

    std::string value = it->second;
    value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
    auto parts = split_whitespace(value);
    if (parts.size() < 3) {
        return def;
    }

    float x, y, z;
    if (!safe_stof(parts[0], x) || !safe_stof(parts[1], y) || !safe_stof(parts[2], z)) {
        return def;
    }

    return glm::vec3(x, y, z);
}

/**
 * Retrieves a float value from a property with a custom default value.
 * Returns the default value if the property is invalid or missing.
 */
float EntityData::GetPropertyFloat(const std::string& propName, float defaultValue) {
    auto it = Properties.find(propName);
    if (it == Properties.end()) {
        return defaultValue;
    }

    std::string value = it->second;
    value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
    auto parts = split_whitespace(value);
    if (parts.empty()) {
        return defaultValue;
    }

    float val;
    if (!safe_stof(parts[0], val)) {
        return defaultValue;
    }

    return val;
}

/**
 * Retrieves a boolean value from a property with a custom default value.
 * Interprets "true"/"1" as true, "false"/"0" as false, otherwise returns default.
 */
bool EntityData::GetPropertyBool(const std::string& propName, bool defaultValue) {
    auto it = Properties.find(propName);
    if (it == Properties.end()) {
        return defaultValue;
    }

    std::string value = it->second;
    value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());
    auto parts = split_whitespace(value);
    if (parts.empty()) {
        return defaultValue;
    }

    std::string boolStr = parts[0];
    std::replace(boolStr.begin(), boolStr.end(), '0', 'f');
    std::replace(boolStr.begin(), boolStr.end(), '1', 't');
    std::transform(boolStr.begin(), boolStr.end(), boolStr.begin(), ::tolower);

    if (boolStr == "true") {
        return true;
    }
    else if (boolStr == "false") {
        return false;
    }
    else {
        return defaultValue;
    }
}

/**
 * Retrieves a string value from a property with a custom default value.
 * Returns the default value if the property is missing.
 */
std::string EntityData::GetPropertyString(const std::string& propName, const std::string& defaultValue) {
    auto it = Properties.find(propName);
    if (it != Properties.end()) {
        return it->second;
    }
    return defaultValue;
}

/**
 * Converts an imported rotation vector to a usable format.
 * Adjusts based on whether it's for a model or not.
 */
glm::vec3 EntityData::ConvertRotation(glm::vec3 importRot, bool notForModel) {
    if (!notForModel) {
        importRot += glm::vec3(0.0f, 180.0f, 0.0f);
    }
    // Convert to radians
    importRot = glm::radians(importRot);

    // Create the rotation matrix with the specified order
    glm::mat4 rotM = glm::rotate(glm::mat4(1.0f), -importRot.z, glm::vec3(1, 0, 0));
    rotM = glm::rotate(rotM, importRot.x, glm::vec3(0, 0, 1));
    rotM = glm::rotate(rotM, importRot.y, glm::vec3(0, 1, 0));

    // Decompose using quaternion conversion
    glm::quat q = glm::quat_cast(rotM);
    glm::vec3 rotation = glm::eulerAngles(q);

    if (notForModel) {
        rotation += glm::vec3(0.0f, glm::radians(90.0f), 0.0f);
    }
    return rotation;
}

// --------------------------
// Global helper function
// --------------------------
EntityData* GetEntityDataFromClass(MapData& mapData, const std::string& className) {
    for (auto& ent : mapData.Entities) {
        if (ent.Classname == className)
            return &ent;
    }
    return nullptr;
}

void MapData::LoadToLevel()
{

    string modelPath = Path.substr(0, Path.length()-3) + "obj";

    for (EntityData entityData : Entities)
    {

        Entity* ent = LevelObjectFactory::instance().create(entityData.Classname);

        if (ent == nullptr)
            ent = new Entity();

        ent->FromData(entityData);

        vector<BrushFaceMesh*> entBrushes;

        vector<RefConst<Shape>> colShapes;

        if (entityData.Brushes.size()) 
        {

            for (BrushData brushData : entityData.Brushes)
            {

                string meshName = "entity" + entityData.name + "_brush" + brushData.Name;

                auto faces = BrushFaceMesh::GetMeshesFromName(modelPath, meshName);

                for (auto face : faces)
                {

                    auto shape = Physics::CreateConvexHullFromPoints(face->vertexLocations);

                    colShapes.push_back(shape);

                }

                

                for (auto face : faces)
                {

                    

                    entBrushes.push_back(face);
                }

            }

            auto compoundShape = Physics::CreateStaticCompoundShapeFromConvexShapes(colShapes);

            ent->LeadBody = Physics::CreateBodyFromShape(ent, vec3(0), compoundShape, 1000, true, BodyType::World | (BodyType::WorldOpaque));

            entBrushes = BrushFaceMesh::MergeMeshesByMaterial(entBrushes);

            for (auto face : entBrushes)
            {
                face->StaticNavigation = ent->Static;
                ent->Drawables.push_back(face);
            }
        }

        Level::Current->AddEntity(ent);

    }

}
