// Explosion.h
#pragma once

#include <Entity.h>
#include <SkeletalMesh.hpp>
#include <vector>
#include <cmath>

enum class ExplosionLayer { Flash, Fireball, InnerFire, Spark, Smoke };

struct ExplosionParticle
{
    StaticMesh* mesh = nullptr;
    vec3            dir;          // unit spread direction
    float           speed;        // max world-unit travel distance
    float           peakSize;     // sphere scale at peak
    float           lifeStart;    // seconds from entity spawn
    float           lifeEnd;      // seconds from entity spawn
    ExplosionLayer  layer;
};

class Explosion : public Entity
{
public:
    Explosion();
    ~Explosion();

    void LoadAssets() override;
    void Start()      override;
    void AsyncUpdate() override;

    float Radius = 1.2f;   // controls spread scale (world units)
    float Lifetime = 2.5f;   // total effect duration in seconds
	float minDamage = 10.f;
	float MaxDamage = 40.f;
    float minPlayerDamage = 10.f;
    float MaxPlayerDamage = 40.f;

	Entity* damageCauser = nullptr; // for damage attribution (e.g. player grenades)

private:
    std::vector<ExplosionParticle> particles;

    void BuildParticles();
    void TickParticle(ExplosionParticle& p, float elapsed);

    // --- Easing ---
    static float EaseOutCubic(float t) { return 1.f - powf(1.f - t, 3.f); }
    static float EaseOutQuad(float t) { return 1.f - powf(1.f - t, 2.f); }
    static float EaseInQuad(float t) { return t * t; }
    static float EaseInCubic(float t) { return t * t * t; }


    // Asymmetric bell: rises to 1 at peakAt, falls back to 0 at t=1
    static float Bell(float t, float peakAt, float riseExp, float fallExp)
    {
        if (t <= 0.f) return 0.f;
        if (t >= 1.f) return 0.f;
        if (t < peakAt)
            return powf(t / peakAt, riseExp);
        else
            return powf(1.f - (t - peakAt) / (1.f - peakAt), fallExp);
    }

    // --- Color ---
    static vec4 FlashColor(float t);
    static vec4 FireballColor(float t);
    static vec4 SmokeColor(float t);
};