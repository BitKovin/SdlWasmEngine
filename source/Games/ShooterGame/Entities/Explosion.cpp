// Explosion.cpp

#include "Explosion.h"
#include <cstdlib>
#include <algorithm>

#include <Entities/Player/Weapons/WeaponFireFlash.h>

#include <Physics.h>

// ---------------------------------------------------------------------------
// Random helpers
// ---------------------------------------------------------------------------

static float RandF(float lo, float hi)
{
    return lo + (hi - lo) * static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

static vec3 RandSphere()
{
    vec3 v;
    float lenSq;
    do {
        v = vec3(RandF(-1.f, 1.f), RandF(-1.f, 1.f), RandF(-1.f, 1.f));
        lenSq = v.x * v.x + v.y * v.y + v.z * v.z;
    } while (lenSq < 0.0001f || lenSq > 1.f);
    return normalize(v);
}

static vec3 RandBiased(vec3 up, float strength)
{
    vec3 v = RandSphere() + up * strength;
    float len = sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
    if (len < 0.0001f) return up;
    return vec3(v.x / len, v.y / len, v.z / len);
}

// ---------------------------------------------------------------------------
// Color functions
// ---------------------------------------------------------------------------

vec4 Explosion::FlashColor(float t)
{
    // 0.0 – 0.15  ignition: warm white, easing in
    // 0.15 – 0.50 bloom: amber-orange, fully opaque
    // 0.50 – 0.75 transition: orange bleeding into brown-grey
    // 0.75 – 1.00 smoke tail: dark grey, slow fade out

    float r, g, b;

    if (t < 0.15f) {
        float u = t / 0.15f;
        r = 1.f;
        g = 0.85f - u * 0.25f;   // white → warm amber
        b = 0.55f - u * 0.55f;   // white → no blue
    }
    else if (t < 0.50f) {
        float u = (t - 0.15f) / 0.35f;
        r = 1.f - u * 0.18f;   // amber → deep orange
        g = 0.60f - u * 0.30f;
        b = 0.f;
    }
    else if (t < 0.75f) {
        float u = (t - 0.50f) / 0.25f;
        r = 0.82f - u * 0.52f;   // orange → brown-grey
        g = 0.30f - u * 0.22f;
        b = u * 0.08f;            // tiny blue creeps in as it goes ashy
    }
    else {
        float u = (t - 0.75f) / 0.25f;
        r = 0.30f - u * 0.18f;   // brown-grey → dark smoke
        g = 0.08f + u * 0.06f;
        b = 0.08f + u * 0.06f;
    }

    // Ease in over first 8%, hold fully opaque through bloom,
    // then a long slow fade that reaches near-zero at t=1
    float alpha;
    if (t < 0.08f)
        alpha = t / 0.08f;
    else if (t < 0.45f)
        alpha = 1.f;
    else
        alpha = powf(1.f - (t - 0.45f) / 0.55f, 1.2f);


    return vec4(r, g, b, alpha) * vec4(0.65, 0.3f, 0.3f, 0.8f);
}

vec4 Explosion::FireballColor(float t)
{
    float r, g;

    if (t < 0.30f) {
        float u = t / 0.30f;
        r = 0.92f;
        g = 0.45f - u * 0.20f;
    }
    else if (t < 0.65f) {
        float u = (t - 0.30f) / 0.35f;
        r = 0.92f - u * 0.42f;
        g = 0.25f - u * 0.22f;
    }
    else {
        float u = (t - 0.65f) / 0.35f;
        r = 0.50f - u * 0.35f;
        g = 0.03f;
    }

    // Ease in over first 12% so fireball breathes in, not pops in.
    // Holds opaque through mid-life, gradual fade after.
    float alpha;
    if (t < 0.12f)
        alpha = t / 0.12f;
    else if (t < 0.55f)
        alpha = 1.f;
    else
        alpha = powf(1.f - (t - 0.55f) / 0.45f, 1.3f);


    return vec4(r, g, 0.f, alpha) * vec4(0.65,0.3f,0.3f,0.1f);
}

vec4 Explosion::SmokeColor(float t)
{
    float base = 0.10f + t * 0.22f;

    // Ease in over first 20% of smoke particle life — no pop-in.
    float rawAlpha = Bell(t, 0.35f, 1.4f, 1.1f) * 0.60f;
    float easeIn = (t < 0.20f) ? (t / 0.20f) : 1.f;
    float alpha = rawAlpha * easeIn;

    return vec4(base, base, base, alpha) * vec4(0.4f, 0.4f, 0.4f,0.9f);
}

void Explosion::NetSerialize(NetPacket& packet)
{
    packet.WriteVector3(Position);
    packet.WriteFloat(Radius);
    packet.WriteFloat(Lifetime);
    packet.WriteFloat(minDamage);
    packet.WriteFloat(MaxDamage);
    packet.WriteFloat(minPlayerDamage);
    packet.WriteFloat(MaxPlayerDamage);
}

void Explosion::NetDeserialize(NetPacket& packet)
{
    Position = packet.ReadVector3();
    Radius = packet.ReadFloat();
    Lifetime = packet.ReadFloat();
    minDamage = packet.ReadFloat();
    MaxDamage = packet.ReadFloat();
    minPlayerDamage = packet.ReadFloat();
    MaxPlayerDamage = packet.ReadFloat();
}

// ---------------------------------------------------------------------------
// Entity
// ---------------------------------------------------------------------------

Explosion::Explosion() { ClassName = "explosion"; }
Explosion::~Explosion() {}

void Explosion::LoadAssets()
{
    BuildParticles();
    for (auto& p : particles)
        Drawables.push_back(p.mesh);
}

void Explosion::BuildParticles()
{
    const char* kMesh = "GameData/models/effects/explosion.glb";
    const char* kTex = "GameData/models/effects/explosion.glb/";

    auto MakeMesh = [&]() -> StaticMesh*
        {
            auto* m = new StaticMesh(this);
            m->LoadFromFile(kMesh);
            m->TexturesLocation = kTex;
            m->blendMode = BgfxStateManager::Blend::Additive;
            m->SetPixelShader("fs_default_simple");
            m->Transparent = true;
            m->DepthWrite = false;
            m->TwoSided = true;
            m->Color = vec4(0.f, 0.f, 0.f, 0.f);
            return m;
        };

    // Flash -> Smoke
    for (int i = 0; i < 3; i++)
    {
        ExplosionParticle p;
        p.mesh = MakeMesh();
        p.dir = RandBiased(vec3(0, 1, 0), 0.3f);

        p.speedMultiplier = RandF(0.50f, 0.7f);
        p.sizeMultiplier = RandF(0.75f, 0.9f);
        p.startFraction = 0.f;
        p.endFraction = 1.0f; // 100% of Lifetime

        p.layer = ExplosionLayer::Flash;
        particles.push_back(p);
    }

    // Fireball
    for (int i = 0; i < 3; i++)
    {
        ExplosionParticle p;
        p.mesh = MakeMesh();
        p.mesh->blendMode = BgfxStateManager::Blend::Additive;
        p.dir = RandBiased(vec3(0, 1, 0), 0.3f);

        p.speedMultiplier = RandF(1.0f, 2.2f) * 0.6f;
        p.sizeMultiplier = RandF(0.40f, 0.80f);
        p.startFraction = 0.f;
        p.endFraction = RandF(0.50f, 0.70f); // 50-70% of Lifetime

        p.layer = ExplosionLayer::Fireball;
        particles.push_back(p);
    }

    // Smoke
    for (int i = 0; i < 4; i++)
    {
        ExplosionParticle p;
        p.mesh = MakeMesh();
        p.dir = RandBiased(vec3(0, 1, 0), 0.85f);

        p.speedMultiplier = RandF(0.80f, 1.40f);
        p.sizeMultiplier = RandF(0.8f, 1.3f);
        p.startFraction = RandF(0.08f, 0.20f); // Starts at 8-20% of Lifetime
        p.endFraction = 1.0f;

        p.layer = ExplosionLayer::Smoke;
        particles.push_back(p);
    }
}

void Explosion::Start()
{
    Entity::Start();
    for (auto& p : particles)
    {
        p.mesh->Position = Position;
        p.mesh->Scale = vec3(0.f, 0.f, 0.f);
    }
    DestroyWithDelay(Lifetime);
	WeaponFireFlash::CreateAt(Position, Lifetime, Radius * 10, 2.5f);

    if (isOwned == false) return;

    bool hasHit = true;

	vector<Entity*> hitEntities;

    std::vector<Physics::HitResult> hits = Physics::MultiSphereOverlap(
        Position,
        Radius,
        BodyType::GroupHitTest,
        {},           // ignoreList (Body*)
        hitEntities   // entityIgnoreList – keeps your existing ignore behavior
    );

    for (const auto& hit : hits)
    {
        if (!hit.hasHit || hit.entity == nullptr)
            continue;

        // Extra safety in case Jolt returns multiple hits on the same body (e.g. compound shapes)
        if (std::find(hitEntities.begin(), hitEntities.end(), hit.entity) != hitEntities.end())
            continue;

        hitEntities.push_back(hit.entity);

        float dist = length(hit.position - Position);

        float damage = MathHelper::MapRange(dist, Radius / 2.0f, Radius, MaxDamage, minDamage);

		if (hit.entity->HasTag("player"))
        {
            damage = MathHelper::MapRange(dist, Radius / 2.0f, Radius, MaxPlayerDamage, minPlayerDamage);
        }




        hit.entity->OnPointDamage(damage, hit.position,
            normalize(hit.position - Position),
            hit.hitboxName, damageCauser, this);

        if (hit.hitbody != nullptr)
        {
            Physics::AddImpulse(hit.hitbody,
                normalize(hit.position - Position) * damage * 10.0f);
        }
    }


}

void Explosion::TickParticle(ExplosionParticle& p, float elapsed)
{
    // Evaluate actual start/end times based on CURRENT Lifetime
    float currentStart = p.startFraction * Lifetime;
    float currentEnd = p.endFraction * Lifetime;

    if (elapsed < currentStart || elapsed > currentEnd)
    {
        p.mesh->Color = vec4(0.f, 0.f, 0.f, 0.f);
        return;
    }

    float duration = currentEnd - currentStart;

    // Safety check to prevent divide-by-zero if Lifetime drops to 0
    float t = (duration > 0.0001f) ? std::max(0.f, std::min(1.f, (elapsed - currentStart) / duration)) : 1.f;

    float tPos = 0.f;
    float tScale = 0.f;
    vec4  color;

    switch (p.layer)
    {
    case ExplosionLayer::Flash:
        tPos = EaseOutCubic(t);
        tScale = (t < 0.20f) ? EaseOutCubic(t / 0.20f) : 1.f;
        tScale += t * 0.25f;
        color = FlashColor(t);
        break;

    case ExplosionLayer::Fireball:
        tPos = t;
        if (t < 0.25f)
            tScale = EaseOutCubic(t / 0.25f);
        else
            tScale = 1.f + (t - 0.25f) * 0.30f;
        color = FireballColor(t);
        break;

    case ExplosionLayer::Smoke:
        tPos = t;
        tScale = EaseOutQuad(t) * 1.2f + t * 0.25f;
        color = SmokeColor(t);
        break;

    default:
        break;
    }

    // Evaluate actual sizes and speeds based on CURRENT Radius
    float currentSpeed = p.speedMultiplier * Radius;
    float currentPeakSize = p.sizeMultiplier * Radius;

    vec3 offset = p.dir * (currentSpeed * tPos) * 0.5f;
    float s = currentPeakSize * tScale;

    p.mesh->Position = Position + offset;
    p.mesh->Scale = vec3(s, s, s);
    p.mesh->Color = color;
}

void Explosion::AsyncUpdate()
{
    float elapsed = Time::GameTime - SpawnTime;
    for (auto& p : particles)
        TickParticle(p, elapsed);

    UpdateDestroyDelay();
}

REGISTER_ENTITY(Explosion, "explosion");