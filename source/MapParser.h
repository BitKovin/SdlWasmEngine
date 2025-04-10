#pragma once

#include <string>
#include <vector>
#include "MapData.h"
#include "glm.h"

// The MapParser namespace wraps functions related to parsing map files.
namespace MapParser {

    // Reset and parse a map file into a MapData object.
    MapData ParseMap(const std::string& pathInput);

    // Utility math functions that mirror the C# implementation.
    glm::vec3 CalculatePlaneNormal(const glm::vec3& point1,
        const glm::vec3& point2,
        const glm::vec3& point3);

    // Given three planes (each defined by an offset and normal), compute their intersection point.
    // Returns true if a unique intersection is found.
    bool GetPlaneContacts(
        const glm::vec3& offset1, const glm::vec3& normal1,
        const glm::vec3& offset2, const glm::vec3& normal2,
        const glm::vec3& offset3, const glm::vec3& normal3,
        glm::vec3& intersectionPoint);

    // Create vertices for a triangulated cube of the given size.
    std::vector<glm::vec3> CreateTriangulatedCubeVertices(float size);
}