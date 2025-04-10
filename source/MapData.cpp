#include "MapData.h"
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <cmath>
#include "glm.h"

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

// -------------------
// EntityData Methods
// -------------------

glm::vec3 EntityData::GetPropertyVectorPosition(const std::string& propName) {
    try {
        auto it = Properties.find(propName);
        if (it == Properties.end())
            throw std::runtime_error("Property not found");

        std::string value = it->second;
        value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());

        auto parts = split_whitespace(value);
        if (parts.size() < 3)
            throw std::runtime_error("Not enough components");

        float x = std::stof(parts[0]);
        float y = std::stof(parts[2]);
        float z = std::stof(parts[1]) * -1.0f;
        return glm::vec3(x, y, z) / MapData::UnitSize;
    }
    catch (...) {
        return glm::vec3(0.0f);
    }
}

glm::vec3 EntityData::GetPropertyVectorRotation(const std::string& propName) {
    try {
        auto it = Properties.find(propName);
        if (it == Properties.end())
            throw std::runtime_error("Property not found");

        std::string value = it->second;
        value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());

        auto parts = split_whitespace(value);
        if (parts.size() < 3)
            throw std::runtime_error("Not enough components");

        float x = std::stof(parts[0]);
        float y = std::stof(parts[1]);
        float z = std::stof(parts[2]);
        return glm::vec3(x, y, z);
    }
    catch (...) {
        return glm::vec3(0.0f);
    }
}

glm::vec3 EntityData::GetPropertyVector(const std::string& propName, const glm::vec3& def) {
    try {
        auto it = Properties.find(propName);
        if (it == Properties.end())
            throw std::runtime_error("Property not found");

        std::string value = it->second;
        value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());

        auto parts = split_whitespace(value);
        if (parts.size() < 3)
            throw std::runtime_error("Not enough components");

        float x = std::stof(parts[0]);
        float y = std::stof(parts[1]);
        float z = std::stof(parts[2]);
        return glm::vec3(x, y, z);
    }
    catch (...) {
        return def;
    }
}

float EntityData::GetPropertyFloat(const std::string& propName, float defaultValue) {
    try {
        auto it = Properties.find(propName);
        if (it == Properties.end())
            throw std::runtime_error("Property not found");

        std::string value = it->second;
        value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());

        auto parts = split_whitespace(value);
        if (parts.empty())
            throw std::runtime_error("Empty property");
        return std::stof(parts[0]);
    }
    catch (...) {
        return defaultValue;
    }
}

bool EntityData::GetPropertyBool(const std::string& propName, bool defaultValue) {
    try {
        auto it = Properties.find(propName);
        if (it == Properties.end())
            throw std::runtime_error("Property not found");

        std::string value = it->second;
        // Replace 0 with false and 1 with true.
        std::replace(value.begin(), value.end(), '0', 'f');
        std::replace(value.begin(), value.end(), '1', 't');
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

        auto parts = split_whitespace(value);
        if (parts.empty())
            throw std::runtime_error("Empty property");
        if (parts[0] == "true")
            return true;
        else if (parts[0] == "false")
            return false;
        else
            throw std::runtime_error("Invalid bool value");
    }
    catch (...) {
        return defaultValue;
    }
}

std::string EntityData::GetPropertyString(const std::string& propName, const std::string& defaultValue) {
    auto it = Properties.find(propName);
    if (it != Properties.end())
        return it->second;
    return defaultValue;
}

glm::vec3 EntityData::ConvertRotation(glm::vec3 importRot, bool notForModel) {
    if (!notForModel) {
        importRot += glm::vec3(0.0f, 180.0f, 0.0f);
    }
    // Convert to radians.
    importRot = glm::radians(importRot);

    // Create the rotation matrix with the order similar to the original.
    glm::mat4 rotM = glm::rotate(glm::mat4(1.0f), -importRot.z, glm::vec3(1, 0, 0));
    rotM = glm::rotate(rotM, importRot.x, glm::vec3(0, 0, 1));
    rotM = glm::rotate(rotM, importRot.y, glm::vec3(0, 1, 0));

    // Decompose using quaternion conversion.
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
