#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "glm.h"
#include <vector>
#include "MathHelper.hpp"  // Your MathHelper functions declared in the RetroEngine namespace


    struct CameraShake;

    class Camera {
    public:
        // Declaration of static members
        static float AspectRatio;
        static vec3 position;
        static vec3 rotation;
        static mat4 Transform;
        static mat4 UiMatrix;

        static mat4 world;
        static mat4 view;
        static mat4 projection;
        static mat4 projectionOcclusion;
        static mat4 projectionViewmodel;

        static mat4 finalizedView;
        static mat4 finalizedProjection;
        static mat4 finalizedProjectionViewmodel;

        static vec3 finalizedPosition;
        static vec3 finalizedRotation;

        static mat4 frustum;
        static mat4 frustumOcclusion;

        static float FOV;
        static float ViewmodelFOV;
        static float FarPlane;

        static vec3 lastWorkingRotation;
        static vec3 velocity;
        static std::vector<CameraShake> CameraShakes;

        // Accessors using MathHelper functions (assumed to be in the RetroEngine namespace)
        static vec3 Up();
        static vec3 Right();
        static vec3 Forward();

        static float GetHorizontalFOV();

        // Camera functions
        static void Update(float deltaTime);
        static void ApplyCameraShake(float deltaTime);
        static void AddCameraShake(const CameraShake& cameraShake);
        static mat4 GetRotationMatrix();
        static mat4 GetMatrix();
        static mat4 CalculateView();
        static void ViewportUpdate(int clientWidth, int clientHeight, float uiViewportHeight);
        static void Follow(const vec3& targetPosition);

    private:
        static void StupidCameraFix();
    };

    // CameraShake struct declaration
    struct CameraShake {
        enum ShakeType { SingleWave, PerlinNoise };

        float duration;
        vec3 positionAmplitude;
        vec3 positionFrequency;
        vec3 rotationAmplitude;
        vec3 rotationFrequency;
        float falloff;
        float elapsedTime;
        float interpIn;
        vec3 currentOffsetPosition;
        vec3 currentOffsetRotation;
        ShakeType shakeType;
        float intensity;

        CameraShake(float interpIn = 0.2f,
            float duration = 1.0f,
            vec3 positionAmplitude = vec3(1.0f),
            vec3 positionFrequency = vec3(10.0f),
            vec3 rotationAmplitude = vec3(1.0f),
            vec3 rotationFrequency = vec3(10.0f),
            float falloff = 1.0f,
            ShakeType shakeType = SingleWave);

        void Update(float deltaTime);
        std::pair<vec3, vec3> GetResult(const vec3& currentPosition, const vec3& currentRotation);
        void Reset();
        bool IsFinished() const;

    private:
        float GetPerlinNoise(float input);
    };

#endif // CAMERA_HPP
