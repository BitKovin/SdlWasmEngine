#include "MapParser.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>

// ----------------------------------------------------------------------
// Helper functions for string manipulation
// ----------------------------------------------------------------------
static std::string trim(const std::string& s) {
    auto start = s.begin();
    while (start != s.end() && std::isspace(*start))
        start++;
    auto end = s.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));
    return std::string(start, end + 1);
}

static std::vector<std::string> split_whitespace(const std::string& s) {
    std::vector<std::string> tokens;
    std::istringstream iss(s);
    std::string token;
    while (iss >> token)
        tokens.push_back(token);
    return tokens;
}

// ----------------------------------------------------------------------
// Global static variables used during parsing.
// ----------------------------------------------------------------------
namespace {
    EntityData* currentEntity = nullptr;
    BrushData* currentBrush = nullptr;
    MapData mapData; // Global mapData instance
}

// ----------------------------------------------------------------------
// Local helper functions to finish an entity or brush.
// ----------------------------------------------------------------------
static void FinishEntity() {
    if (currentEntity) {
        mapData.Entities.push_back(*currentEntity);
    }
}

static void FinishBrush() {
    if (currentEntity && currentBrush) {
        currentEntity->Brushes.push_back(*currentBrush);
    }
}

// Dummy AssetRegistry placeholder
static std::string FindPathForFile(const std::string& path) {
    // Implement proper asset lookup logic if needed.
    return path;
}

// ----------------------------------------------------------------------
// Implementation of MapParser functions.
// ----------------------------------------------------------------------
namespace MapParser {

    MapData ParseMap(const std::string& pathInput) 
    {
        // Reset global state.
        currentEntity = nullptr;
        currentBrush = nullptr;
        mapData = MapData();

        std::string path = FindPathForFile(pathInput);
        mapData.Path = path;

        std::ifstream file(path);
        if (!file.is_open()) {
            std::cerr << "Unable to open file: " << path << std::endl;
            return mapData;
        }

        std::string line;
        while (std::getline(file, line)) {
            line = trim(line);
            if (line.empty())
                continue;

            // Remove quotes.
            line.erase(std::remove(line.begin(), line.end(), '\"'), line.end());

            auto parts = split_whitespace(line);

            if (line.find("// entity") == 0) {
                currentEntity = new EntityData();
                if (parts.size() >= 3)
                    currentEntity->name = parts[2];
            }
            else if (line.find("classname") == 0) {
                if (currentEntity && parts.size() >= 2)
                    currentEntity->Classname = parts[1];
            }
            else if (line.find("// brush") == 0) {
                currentBrush = new BrushData();
                if (parts.size() >= 3)
                    currentBrush->Name = parts[2];
            }
            else if (line.find("(") == 0) {
                // In the original, this block was empty.
            }
            else if (line.find("}") == 0) {
                if (currentBrush) {
                    FinishBrush();
                    delete currentBrush;
                    currentBrush = nullptr;
                }
                else if (currentEntity) {
                    FinishEntity();
                    delete currentEntity;
                    currentEntity = nullptr;
                }
            }
            else if (line.find("//") == 0 || line.find("{") == 0) {
                // Skip comments or opening brace.
                continue;
            }
            else if (parts.size() > 1) {
                if (currentEntity) {
                    std::string key = parts[0];
                    // Remove key and following space from line.
                    std::string value = line.substr(key.size() + 1);
                    currentEntity->Properties[key] = value;
                }
            }
        }

        file.close();
        return mapData;
    }

    glm::vec3 CalculatePlaneNormal(const glm::vec3& point1,
        const glm::vec3& point2,
        const glm::vec3& point3) {
        glm::vec3 vector1 = point2 - point1;
        glm::vec3 vector2 = point3 - point1;
        return glm::normalize(glm::cross(vector1, vector2));
    }

    bool GetPlaneContacts(
        const glm::vec3& offset1, const glm::vec3& normal1,
        const glm::vec3& offset2, const glm::vec3& normal2,
        const glm::vec3& offset3, const glm::vec3& normal3,
        glm::vec3& intersectionPoint)
    {
        glm::vec3 cross1_2 = glm::cross(normal1, normal2);
        glm::vec3 cross2_3 = glm::cross(normal2, normal3);
        float determinant = glm::dot(normal1, cross2_3);
        if (std::abs(determinant) < 1e-6f) {
            intersectionPoint = glm::vec3(0.0f);
            return false;
        }
        glm::vec3 distance = offset2 - offset1;
        float t1 = glm::dot(distance, cross2_3) / determinant;
        intersectionPoint = offset1 + normal1 * t1;
        return true;
    }

    std::vector<glm::vec3> CreateTriangulatedCubeVertices(float size) {
        float halfSize = size / 2.0f;
        std::vector<glm::vec3> vertices = {
            // Front face
            glm::vec3(-halfSize, halfSize, halfSize),
            glm::vec3(halfSize,  halfSize, halfSize),
            glm::vec3(halfSize, -halfSize, halfSize),
            glm::vec3(-halfSize,-halfSize, halfSize),
            // Back face
            glm::vec3(-halfSize, halfSize, -halfSize),
            glm::vec3(halfSize,  halfSize, -halfSize),
            glm::vec3(halfSize, -halfSize, -halfSize),
            glm::vec3(-halfSize,-halfSize, -halfSize)
        };

        std::vector<glm::vec3> cubeVertices;
        int indices[] = {
             0, 1, 2, 2, 3, 0, // Front
             4, 5, 6, 6, 7, 4, // Back
             0, 4, 7, 7, 3, 0, // Left
             1, 5, 6, 6, 2, 1, // Right
             0, 1, 5, 5, 4, 0, // Top
             2, 3, 7, 7, 6, 2  // Bottom
        };

        for (int idx : indices) {
            cubeVertices.push_back(vertices[idx]);
        }
        return cubeVertices;
    }

} // namespace MapParser
