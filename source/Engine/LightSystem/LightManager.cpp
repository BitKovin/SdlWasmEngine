#include "LightManager.h"

#include "../Camera.h"

#include "../BoundingSphere.hpp"

#include "../DebugDraw.hpp"

vec3 LightManager::LightDirection = vec3(-1,-1,1);

const float cascadeExponent = 3.0f;

float LightManager::LightDistance = 200;

float LightManager::LightDistance1 = 0;
float LightManager::LightDistance2 = 0;
float LightManager::LightDistance3 = 0;
float LightManager::LightDistance4 = 0;

float LightManager::LightRadius1 = 0;
float LightManager::LightRadius2 = 0;
float LightManager::LightRadius3 = 0;
float LightManager::LightRadius4 = 0;

int LightManager::ShadowMapResolution = 1024*2;
float LightManager::LightDistanceMultiplier = 1;
float LightManager::ShaddowOffsetScale = 1;

mat4 LightManager::lightView1;
mat4 LightManager::lightProjection1;

mat4 LightManager::lightView2;
mat4 LightManager::lightProjection2;

mat4 LightManager::lightView3;
mat4 LightManager::lightProjection3;

mat4 LightManager::lightView4;
mat4 LightManager::lightProjection4;

bool LightManager::DirectionalShadowsEnabled = false;

LightManager::LightManager()
{
}

LightManager::~LightManager()
{
}


std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
    const auto inv = glm::inverse(proj * view);

    std::vector<glm::vec4> frustumCorners;
    for (unsigned int x = 0; x < 2; ++x)
    {
        for (unsigned int y = 0; y < 2; ++y)
        {
            for (unsigned int z = 0; z < 2; ++z)
            {
                const glm::vec4 pt =
                    inv * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f);

                frustumCorners.push_back(pt / pt.w);
            }
        }
    }

    return frustumCorners;
}

void DebugDrawCorders(std::vector<glm::vec4> points)
{
    for (auto p : points)
    {
        DebugDraw::Line(Camera::position, p, 3);
    }
}

BoundingSphere CalculateCascadeInfo(const glm::mat4& proj, const glm::mat4& view)
{
    auto corners = getFrustumCornersWorldSpace(proj, view);

    return BoundingSphere::FromPoints(corners);

}

void LightManager::Update()
{
    // 1) Tweakable parameters
    LightDirection = glm::normalize(LightDirection);
    
    LightDistance4 = LightDistance;
    LightDistance3 = LightDistance4 / cascadeExponent;
    LightDistance2 = LightDistance3 / cascadeExponent;
    LightDistance1 = LightDistance2 / cascadeExponent;

    mat4 view = Camera::finalizedView;
    mat4 proj1 = perspective(radians(Camera::FOV), Camera::AspectRatio, 0.05f, LightDistance1);
    mat4 proj2 = perspective(radians(Camera::FOV), Camera::AspectRatio, LightDistance1, LightDistance2);
    mat4 proj3 = perspective(radians(Camera::FOV), Camera::AspectRatio, LightDistance2, LightDistance3);
    mat4 proj4 = perspective(radians(Camera::FOV), Camera::AspectRatio, LightDistance3, LightDistance4);


    BoundingSphere casc1 = CalculateCascadeInfo(proj1, view);
    BoundingSphere casc2 = CalculateCascadeInfo(proj2, view);
    BoundingSphere casc3 = CalculateCascadeInfo(proj3, view);
    BoundingSphere casc4 = CalculateCascadeInfo(proj4, view);

    LightRadius1 = casc1.Radius;
    LightRadius2 = casc2.Radius;
    LightRadius3 = casc3.Radius;
    LightRadius4 = casc4.Radius;

    CalculateLightMatrices(casc1.Radius, casc1.offset,lightView1, lightProjection1);
    CalculateLightMatrices(casc2.Radius, casc2.offset, lightView2, lightProjection2);
    CalculateLightMatrices(casc3.Radius, casc3.offset, lightView3, lightProjection3);
    CalculateLightMatrices(casc4.Radius, casc4.offset, lightView4, lightProjection4);

    FinishPointLights();

}

void LightManager::UpdateLightSource(PointLightInfo light)
{

    m_lights.push_back(light);

}

void LightManager::ApplyPointLightToShader(Shader* shader, vec3 boundsMin, vec3 boundsMax)
{
    if (!shader) return;

    const float minX = boundsMin.x, minY = boundsMin.y, minZ = boundsMin.z;
    const float maxX = boundsMax.x, maxY = boundsMax.y, maxZ = boundsMax.z;

    constexpr size_t MAX_LIGHTS = 16u;
    const int maxLights = static_cast<int>(MAX_LIGHTS);

    // mat4 column layout:
    //   [0] = vec4(position.xyz, innerCos)
    //   [1] = vec4(color.rgb,    radius)
    //   [2] = vec4(direction.xyz, outerCos)
    //   [3] = vec4(reserved)
    std::vector<mat4> pointLights;
    pointLights.assign(MAX_LIGHTS, mat4(0.0f));

    const float DEG2RAD = 3.14159265358979323846f / 180.0f;
    const float MAX_SAFE_DEG = 179.999f;
    const float EPS_LEN_SQ = 1e-12f;

    float added = 0;
    const auto& lights = m_finalLights;
    const size_t nLights = lights.size();

    for (size_t i = 0; i < nLights && added < maxLights; ++i)
    {
        const auto& L = lights[i];

        // AABB-sphere intersection
        const float cx = L.position.x, cy = L.position.y, cz = L.position.z;
        const float closestX = (cx < minX) ? minX : ((cx > maxX) ? maxX : cx);
        const float closestY = (cy < minY) ? minY : ((cy > maxY) ? maxY : cy);
        const float closestZ = (cz < minZ) ? minZ : ((cz > maxZ) ? maxZ : cz);
        const float dx = closestX - cx, dy = closestY - cy, dz = closestZ - cz;
        const float distSq = dx * dx + dy * dy + dz * dz;
        const float radius = L.radius;
        if (distSq > radius * radius) continue;

        // cone sanitization
        float innerDeg = L.innerConeAngleDegrees;
        float outerDeg = L.outerConeAngleDegrees;
        if (innerDeg >= 360.0f) innerDeg = MAX_SAFE_DEG; else if (innerDeg < 0.0f) innerDeg = 0.0f;
        if (outerDeg >= 360.0f) outerDeg = MAX_SAFE_DEG; else if (outerDeg < 0.0f) outerDeg = 0.0f;
        if (innerDeg > outerDeg) { float t = innerDeg; innerDeg = outerDeg; outerDeg = t; }

        float innerCos = cosf(innerDeg * DEG2RAD);
        float outerCos = cosf(outerDeg * DEG2RAD);
        if (innerCos < outerCos) { float t = innerCos; innerCos = outerCos; outerCos = t; }

        // normalize direction
        vec3 dir = L.direction;
        const float lenSq = dir.x * dir.x + dir.y * dir.y + dir.z * dir.z;
        if (lenSq > EPS_LEN_SQ)
        {
            const float invLen = 1.0f / sqrtf(lenSq);
            dir.x *= invLen; dir.y *= invLen; dir.z *= invLen;
        }
        else { dir.x = 0.0f; dir.y = 0.0f; dir.z = 1.0f; }

        // pack into mat4 columns
        mat4& m = pointLights[added];
        m[0] = vec4(L.position.x, L.position.y, L.position.z, innerCos);
        m[1] = vec4(L.color.r, L.color.g, L.color.b, radius);
        m[2] = vec4(dir.x, dir.y, dir.z, outerCos);
        m[3] = vec4(0.0f);  // reserved

        ++added;
    }

    shader->SetUniform("PointLightsNumber", vec4(added));
    shader->SetUniform("PointLights", pointLights);
}


void LightManager::CalculateLightMatrices(
    float lightDistance, glm::vec3 cameraPos,
    glm::mat4& outLightView,
    glm::mat4& outLightProjection) 
{

    glm::vec3 L = glm::normalize(LightDirection);
    float halfSize = lightDistance;
    float nearPlane = -1000.0f;
    float farPlane = lightDistance;
    float shadowResH = ShadowMapResolution * 0.5f;

    // 2) build raw view + ortho
    glm::mat4 view = glm::lookAt(
        cameraPos,
        cameraPos + L,
        glm::vec3(0, 0, 1)
    );
    glm::mat4 proj = glm::ortho(
        -halfSize, +halfSize,
        -halfSize, +halfSize,
        nearPlane, farPlane
    );

    // 3) snap-to-texel-grid
    glm::vec4 originLS = view * glm::vec4(0, 0, 0, 1);
    float worldPerTexel = (2.0f * halfSize) / shadowResH;
    float snappedX = std::floor(originLS.x / worldPerTexel) * worldPerTexel;
    float snappedY = std::floor(originLS.y / worldPerTexel) * worldPerTexel;
    glm::vec3 offset(
        snappedX - originLS.x,
        snappedY - originLS.y,
        0.0f
    );
    view = glm::translate(glm::mat4(1.0f), offset) * view;

    // 4) output
    outLightView = view;
    outLightProjection = proj;
}

void LightManager::FinishPointLights()
{

    m_finalLights.clear();


    for (auto& light : m_lights)
    {

        if (Camera::frustum.IsSphereVisible(light.position, light.radius))
        {

            m_finalLights.push_back(light);

        }

    }

    std::sort(m_finalLights.begin(), m_finalLights.end(),
        [](PointLightInfo a, PointLightInfo b) {
            return distance2(a.position,Camera::position) < distance2(b.position, Camera::position);
        });

    m_lights.clear();

}


