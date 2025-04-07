#pragma once

#include "glm.h"
#include <vector>
#include <cmath>

using namespace std;

class BoudingSphere
{
public:
    BoudingSphere() : offset(vec3(0.0f)), Radius(0.0f) {}
    BoudingSphere(vec3 pos, float radius)
    {
        offset = pos;
        Radius = radius;
    }
    ~BoudingSphere() {}

    vec3 offset; // center of the sphere
    float Radius; // sphere radius

    // Computes a bounding sphere that encloses all given points.
    // Uses a variant of Ritter's algorithm.
    static BoudingSphere FromPoints(const vector<vec3>& points)
    {
        BoudingSphere sphere;

        if (points.empty())
            return sphere;

        // Step 1: Pick an arbitrary point p
        const vec3& p = points[0];

        // Step 2: Find the point q farthest from p
        size_t indexQ = 0;
        float maxDistSq = 0.0f;
        for (size_t i = 1; i < points.size(); i++)
        {
            float d = glm::length2(points[i] - p);
            if (d > maxDistSq)
            {
                maxDistSq = d;
                indexQ = i;
            }
        }
        const vec3& q = points[indexQ];

        // Step 3: Find the point r farthest from q
        size_t indexR = indexQ;
        maxDistSq = 0.0f;
        for (size_t i = 0; i < points.size(); i++)
        {
            float d = glm::length2(points[i] - q);
            if (d > maxDistSq)
            {
                maxDistSq = d;
                indexR = i;
            }
        }
        const vec3& r = points[indexR];

        // Step 4: Set the initial sphere to have its center at the midpoint of q and r, 
        // with radius half the distance between them.
        vec3 center = (q + r) * 0.5f;
        float radius = glm::sqrt(maxDistSq) * 0.5f;

        // Step 5: Expand the sphere to include all points.
        for (const vec3& pt : points)
        {
            vec3 diff = pt - center;
            float dist = glm::length(diff);
            if (dist > radius)
            {
                float newRadius = (radius + dist) * 0.5f;
                // Compute how much to move the center toward the point.
                float k = (newRadius - radius) / dist;
                center += diff * k;
                radius = newRadius;
            }
        }

        sphere.offset = center;
        sphere.Radius = radius;
        return sphere;
    }

    BoudingSphere Transform(vec3 translation, vec3 scale = vec3(1))
    {

        float s = glm::max(glm::max(scale.x, scale.y), scale.z);

        return BoudingSphere(offset * s + translation, Radius * s);
    }



private:

};
