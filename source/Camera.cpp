#include "Camera.h"
#include "glm.h"
#include <cmath>
#include <limits>
#include "MathHelper.hpp"
#include <SDL2/SDL_video.h>


    // Define static members (one definition per static member)
    float Camera::AspectRatio = 1.7777f;
    vec3 Camera::position = vec3(0.0f);
    vec3 Camera::rotation = vec3(45.0f, 0.0f, 0.0f);
    mat4 Camera::Transform = mat4(1.0f);
    mat4 Camera::UiMatrix = mat4(1.0f);
    mat4 Camera::world = mat4(1.0f);
    mat4 Camera::view = mat4(1.0f);
    mat4 Camera::projection = mat4(1.0f);
    mat4 Camera::projectionOcclusion = mat4(1.0f);
    mat4 Camera::projectionViewmodel = mat4(1.0f);
    mat4 Camera::finalizedView = mat4(1.0f);
    mat4 Camera::finalizedProjection = mat4(1.0f);
    mat4 Camera::finalizedProjectionViewmodel = mat4(1.0f);
    vec3 Camera::finalizedPosition = vec3(0.0f);
    vec3 Camera::finalizedRotation = vec3(0.0f);
    Frustum Camera::frustum = Frustum(mat4(1.0f));
    float Camera::FOV = 80.0f;
    float Camera::ViewmodelFOV = 60.0f;
    float Camera::FarPlane = 3000.0f;

    int Camera::ScreenHeight = 720;

    vec3 Camera::lastWorkingRotation = vec3(0.0f);
    vec3 Camera::velocity = vec3(0.0f);
    std::vector<CameraShake> Camera::CameraShakes;

    vec3 Camera::Up() {
        return MathHelper::GetUpVector(rotation);
    }
    vec3 Camera::Right() {
        return MathHelper::GetRightVector(rotation);
    }
    vec3 Camera::Forward() {
        return MathHelper::GetForwardVector(rotation);
    }

    float Camera::GetHorizontalFOV() {
        return FOV * AspectRatio;
    }

    void Camera::StupidCameraFix() {
        if (rotation.x == 0.0f)
            rotation.x = std::numeric_limits<float>::epsilon();
        if (std::fmod(std::fabs(rotation.y), 90.0f) < 0.0001f)
            rotation.y += 0.0001f * (rotation.y > 0.0f ? 1.0f : -1.0f);
        if (rotation.z == 0.0f)
            rotation.z = std::numeric_limits<float>::epsilon();
    }

    mat4 Camera::CalculateView() {
        return lookAt(finalizedPosition,
            finalizedPosition + MathHelper::GetForwardVector(finalizedRotation),
            MathHelper::GetUpVector(finalizedRotation));
    }

    mat4 Camera::GetRotationMatrix() {
        StupidCameraFix();
        return MathHelper::GetRotationMatrix(rotation);
    }

    mat4 Camera::GetMatrix() {
        StupidCameraFix();
        return MathHelper::GetRotationMatrix(rotation) * translate(mat4(1.0f), position);
    }

    void Camera::Update(float deltaTime) {
        finalizedPosition = position;
        finalizedRotation = rotation;
        world = translate(mat4(1.0f), vec3(0.0f));
        if (MathHelper::GetUpVector(rotation).y > 0.0f) {
            lastWorkingRotation = rotation;
        }
        else {
            rotation += vec3(0.0001f, 0.0001f, 0.0f);
        }

        StupidCameraFix();
        view = CalculateView();
        projection = perspective(radians(FOV), AspectRatio, 0.05f, FarPlane);

        projectionOcclusion = perspective(radians(FOV * 1.3f), AspectRatio, 0.05f, FarPlane);

        projectionViewmodel = perspective(radians(ViewmodelFOV), AspectRatio, 0.01f, 1.0f);

        frustum = projection * view;
        finalizedView = view;
        finalizedProjection = projection;
        finalizedProjectionViewmodel = projectionViewmodel;
    }

    void Camera::ApplyCameraShake(float deltaTime) {
        for (auto it = CameraShakes.begin(); it != CameraShakes.end();) {
            it->Update(deltaTime);
            auto result = it->GetResult(position, rotation);
            position = result.first;
            rotation = result.second;
            if (it->IsFinished())
                it = CameraShakes.erase(it);
            else
                ++it;
        }
    }

    void Camera::AddCameraShake(const CameraShake& cameraShake) {
        CameraShakes.push_back(cameraShake);
    }

    void Camera::ViewportUpdate(int clientWidth, int clientHeight, float uiViewportHeight) {
        float newHtW = static_cast<float>(clientWidth) / static_cast<float>(clientHeight);
        if (std::isnan(newHtW) || std::isinf(newHtW))
            newHtW = 1.7777f;
        AspectRatio = newHtW;
        float scaleY = static_cast<float>(clientHeight) / uiViewportHeight;
        UiMatrix = scale(mat4(1.0f), vec3(scaleY, scaleY, 1.0f));
    }

    void Camera::Follow(const vec3& targetPosition) {
        position = targetPosition;
    }

    // --- CameraShake Implementation ---

    CameraShake::CameraShake(float interpIn, float duration,
        vec3 positionAmplitude,
        vec3 positionFrequency,
        vec3 rotationAmplitude,
        vec3 rotationFrequency,
        float falloff,
        ShakeType shakeType)
        : interpIn(interpIn), duration(duration), positionAmplitude(positionAmplitude),
        positionFrequency(positionFrequency), rotationAmplitude(rotationAmplitude),
        rotationFrequency(rotationFrequency), falloff(falloff), elapsedTime(0.0f),
        currentOffsetPosition(vec3(0.0f)), currentOffsetRotation(vec3(0.0f)),
        shakeType(shakeType), intensity(0.0f)
    {
    }

    void CameraShake::Update(float deltaTime) {
        if (elapsedTime >= duration) {
            currentOffsetPosition = vec3(0.0f);
            currentOffsetRotation = vec3(0.0f);
            return;
        }
        float normalizedTime = elapsedTime / duration;
        float interpTime = MathHelper::Saturate(elapsedTime / interpIn);
        intensity = MathHelper::Lerp(1.0f, 0.0f, normalizedTime) * (1.0f - normalizedTime * falloff) * interpTime;
        if (shakeType == ShakeType::SingleWave) {
            currentOffsetPosition = vec3(
                intensity * positionAmplitude.x * std::sin(elapsedTime * positionFrequency.x),
                intensity * positionAmplitude.y * std::sin(elapsedTime * positionFrequency.y),
                intensity * positionAmplitude.z * std::sin(elapsedTime * positionFrequency.z)
            );
            currentOffsetRotation = vec3(
                intensity * rotationAmplitude.x * std::cos(elapsedTime * rotationFrequency.x),
                intensity * rotationAmplitude.y * std::cos(elapsedTime * rotationFrequency.y),
                intensity * rotationAmplitude.z * std::cos(elapsedTime * rotationFrequency.z)
            );
        }
        else if (shakeType == ShakeType::PerlinNoise) {
            currentOffsetPosition = vec3(
                intensity * positionAmplitude.x * GetPerlinNoise(elapsedTime * positionFrequency.x),
                intensity * positionAmplitude.y * GetPerlinNoise(elapsedTime * positionFrequency.y),
                intensity * positionAmplitude.z * GetPerlinNoise(elapsedTime * positionFrequency.z)
            );
            currentOffsetRotation = vec3(
                intensity * rotationAmplitude.x * GetPerlinNoise(elapsedTime * rotationFrequency.x),
                intensity * rotationAmplitude.y * GetPerlinNoise(elapsedTime * rotationFrequency.y),
                intensity * rotationAmplitude.z * GetPerlinNoise(elapsedTime * rotationFrequency.z)
            );
        }
        elapsedTime += deltaTime;
    }

    std::pair<vec3, vec3> CameraShake::GetResult(const vec3& currentPosition, const vec3& currentRotation) {
        vec3 newPosition = currentPosition +
            (currentOffsetPosition.x * MathHelper::GetRightVector(currentRotation) +
                currentOffsetPosition.y * MathHelper::GetUpVector(currentRotation) +
                currentOffsetPosition.z * MathHelper::GetForwardVector(currentRotation));
        vec3 newRotation = currentRotation + currentOffsetRotation;
        return std::make_pair(newPosition, newRotation);
    }

    void CameraShake::Reset() {
        elapsedTime = 0.0f;
        currentOffsetPosition = vec3(0.0f);
        currentOffsetRotation = vec3(0.0f);
    }

    bool CameraShake::IsFinished() const {
        return elapsedTime >= duration;
    }

    float CameraShake::GetPerlinNoise(float input) {
        return std::sin(input) * 0.5f + 0.5f;
    }

