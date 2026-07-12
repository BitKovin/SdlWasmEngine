// ─────────────────────────────────────────────────────────────────────────────
// This is the ONLY .cpp in the project that includes vaudio.h. See the top of
// SpatialSoundManager.h for why. If vaudio's license/SDK ever needs to be
// pulled from the project, this is the one file that needs to change (down to
// a stub implementing the same public functions as no-ops) — nothing that
// includes SpatialSoundManager.h needs to change at all.
// ─────────────────────────────────────────────────────────────────────────────

#include "SpatialSoundManager.h"

// Define this project-wide (e.g. as a preprocessor definition in your build
// config) to fully disable the vaudio-backed spatial audio system: no vaudio
// calls are made, and every query below returns a neutral value, so callers
// apply NO effect at all — sounds play back exactly as if this system didn't
// exist. Combined with the vaudio.h isolation above, this also means the
// vaudio SDK/lib itself can be entirely absent from the build when this is
// defined.
// #define DISABLE_VAUDIO

#ifndef DISABLE_VAUDIO

#include <algorithm>
#include <cmath>
#include <unordered_map>

#include <glm.h>
#include <Entity.h>
#include <Level.hpp>
#include <Camera.h>
#include <MathHelper.hpp>
#include <Logger.hpp>

#include "../BSP/Quake3Bsp.h"
#include "MaterialFromTexture.h"

#include <vaudio.h>

// ─────────────────────────────────────────────────────────────────────────────
// Internal state — deliberately not declared in the header at all (see the
// header's top-of-file note). A purely-static class needs no instance state,
// so this lives as file-local statics instead of private members.
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
    struct EntityAudioGeometry
    {
        std::string entityId;
        std::vector<VAMeshPrimitive*> primitives;
        BSPModelRef* modelRef;

        // Local-space (pre-transform) triangle soup handed to
        // vaMeshPrimitiveCreate() for each entry in 'primitives', kept only
        // for SetDebugDrawGeometry() — vaudio has no accessor to read a
        // primitive's vertices back out, so if we want to draw them we have
        // to hold on to our own copy. Parallel array to 'primitives'. This
        // roughly doubles the memory this system holds per level; if that
        // ever matters, gate the push_back in BuildEntityGeometry behind
        // g_debugDrawGeometry and only support enabling debug draw before
        // BuildWorld() runs.
        std::vector<std::vector<VAVector>> primitiveLocalVerts;
    };

    VAWorld* g_world = nullptr;
    VAEmitter* g_listener = nullptr;
    std::vector<EntityAudioGeometry> g_trackedEntities;

    bool g_debugDrawGeometry = true;

    // Upper bound on how many distinct "room" reverb clusters vaudio's
    // Grouped EAX system tracks simultaneously (see BuildWorld() and
    // CreateSoundEmitter() below — this feeds SpatialReverb::direction).
    // Tune this against how many concurrent 3D sounds + distinct rooms a
    // level realistically has; higher costs more per-frame grouping work.
    constexpr int kMaxGroupedEAXCount = 8;

    // Blends every Grouped EAX cluster's direction relative to g_listener
    // into one pan vector, weighted by each cluster's relative gain. A
    // cluster with no data yet for this listener (gain/direction both null —
    // see vaEaxReverbGetRelativeGain docs) is skipped rather than treated as
    // zero, so a still-initializing room doesn't drag the blend towards
    // "diffuse" before it has real data.
    glm::vec3 ComputeRelativeReverbDirection()
    {
        if (!g_world || !g_listener) return glm::vec3(0.0f);

        int count = vaWorldGetGroupedEAXCount(g_world);
        const VAEAXReverb** grouped = vaWorldGetGroupedEAX(g_world);
        if (count <= 0 || !grouped) return glm::vec3(0.0f);

        glm::vec3 blended(0.0f);
        for (int i = 0; i < count; i++)
        {
            const float* gain = vaEaxReverbGetRelativeGain(grouped[i], g_listener);
            const VAVector* dir = vaEaxReverbGetRelativeDirection(grouped[i], g_listener);
            if (!gain || !dir) continue; // not calculated for this listener yet

            blended += glm::vec3(dir->x, dir->y, dir->z) * (*gain);
        }

        // Direction magnitude feeds AL_EAXREVERB_*_PAN, which expects 0..1 —
        // clamp rather than normalize so a genuinely diffuse blend (multiple
        // rooms roughly canceling out) stays close to (0,0,0) instead of
        // being renormalized back up to full strength in some arbitrary
        // direction.
        float lenSq = blended.x * blended.x + blended.y * blended.y + blended.z * blended.z;
        if (lenSq > 1.0f)
            blended /= std::sqrt(lenSq);

        return blended;
    }

    // ── Small helpers ────────────────────────────────────────────────────────

    // vaWorldAddPrimitive() / vaWorldRemovePrimitive() in vaudio.h are C11
    // _Generic macros — not valid C++. We call the untyped functions they
    // would have expanded to directly instead.
    inline void AddPrimitiveToWorld(VAWorld* w, VAMeshPrimitive* p) { vaWorldAddPrimitive_(w, (void*)p); }
    inline void RemovePrimitiveFromWorld(VAWorld* w, VAMeshPrimitive* p) { vaWorldRemovePrimitive_(w, (void*)p); }

    // VAMatrix's layout (four groups of four floats, column-major) is
    // identical to glm::mat4's — this mapping mirrors what
    // vaVectorMultiplyMatrix() expects: column I's four components are
    // mI1..mI4. Built through vaMatrixCreate() rather than a raw memcpy so it
    // stays correct even if the engine's mat4 wrapper ever changes layout.
    VAMatrix ToVAMatrix(const mat4& m)
    {
        return vaMatrixCreate(
            m[0][0], m[0][1], m[0][2], m[0][3],
            m[1][0], m[1][1], m[1][2], m[1][3],
            m[2][0], m[2][1], m[2][2], m[2][3],
            m[3][0], m[3][1], m[3][2], m[3][3]);
    }

    inline VAVector ToVAVector(const vec3& v) { return vaVectorCreate(v.x, v.y, v.z); }

    inline SpatialSoundEmitter* ToOpaque(VAEmitter* e) { return reinterpret_cast<SpatialSoundEmitter*>(e); }
    inline VAEmitter* FromOpaque(SpatialSoundEmitter* e) { return reinterpret_cast<VAEmitter*>(e); }

    VAMatrix ComputeEntityTransform(BSPModelRef* modelRef)
    {
        mat4 worldMatrix = translate(modelRef->Position) * MathHelper::GetRotationMatrix(modelRef->Rotation) * scale(modelRef->Scale);
        return ToVAMatrix(worldMatrix);
    }

    VAMaterialType ToVAMaterial(SpatialMaterial m)
    {
        switch (m)
        {
        case SpatialMaterial::Air:              return VAMaterialAir;
        case SpatialMaterial::Brick:             return VAMaterialBrick;
        case SpatialMaterial::Cloth:             return VAMaterialCloth;
        case SpatialMaterial::Concrete:          return VAMaterialConcrete;
        case SpatialMaterial::ConcretePolished:  return VAMaterialConcretePolished;
        case SpatialMaterial::Dirt:              return VAMaterialDirt;
        case SpatialMaterial::Glass:             return VAMaterialGlass;
        case SpatialMaterial::Grass:             return VAMaterialGrass;
        case SpatialMaterial::Gravel:            return VAMaterialGravel;
        case SpatialMaterial::Gyprock:           return VAMaterialGyprock;
        case SpatialMaterial::Ice:               return VAMaterialIce;
        case SpatialMaterial::Leaf:              return VAMaterialLeaf;
        case SpatialMaterial::Marble:            return VAMaterialMarble;
        case SpatialMaterial::Metal:             return VAMaterialMetal;
        case SpatialMaterial::Mud:               return VAMaterialMud;
        case SpatialMaterial::Rock:               return VAMaterialRock;
        case SpatialMaterial::Sand:              return VAMaterialSand;
        case SpatialMaterial::Snow:              return VAMaterialSnow;
        case SpatialMaterial::Tile:              return VAMaterialTile;
        case SpatialMaterial::Tree:              return VAMaterialTree;
        case SpatialMaterial::Water:             return VAMaterialWater;
        case SpatialMaterial::WoodIndoor:        return VAMaterialWoodIndoor;
        case SpatialMaterial::WoodOutdoor:       return VAMaterialWoodOutdoor;
        default:                                 return VAMaterialConcrete;
        }
    }

    SpatialLowPassFilter ToSpatialFilter(const VALowPassFilter* f)
    {
        if (!f) return SpatialLowPassFilter{};
        return SpatialLowPassFilter{ f->gainLF, f->gainHF };
    }

    void ToSpatialReverb(const VAEAXReverb& eax, SpatialReverb& out)
    {
        out.density = eax.density;
        out.diffusion = eax.diffusion;
        out.gain = eax.gain;
        out.gainHF = eax.gainHF;
        out.gainLF = eax.gainLF;
        out.decayTime = eax.decayTime;
        out.decayHFRatio = eax.decayHFRatio;
        out.decayLFRatio = eax.decayLFRatio;
        out.reflectionsGain = eax.reflectionsGain;
        out.reflectionsDelay = eax.reflectionsDelay;
        out.lateReverbGain = eax.lateReverbGain;
        out.lateReverbDelay = eax.lateReverbDelay;
        out.echoTime = eax.echoTime;
        out.echoDepth = eax.echoDepth;
        out.modulationTime = eax.modulationTime;
        out.modulationDepth = eax.modulationDepth;
        out.airAbsorptionGainHF = eax.airAbsorptionGainHF;
        out.hfReference = eax.hfReference;
        out.lfReference = eax.lfReference;
        out.roomRolloffFactor = eax.roomRolloffFactor;
        out.decayHFLimit = eax.decayHFLimit;
    }

    // ── World-building ───────────────────────────────────────────────────────

    void BuildEntityGeometry(Entity* entity, BSPModelRef* modelRef)
    {
        tBSPModel& model = modelRef->model;

        // Skip invisible trigger/clip-only "box" brushes (n_faces == 0, see
        // the AABB special case in AddPhysicsBodyForEntityAndModel) — these
        // are gameplay volumes, not real geometry, and shouldn't occlude or
        // reflect sound.
        if (model.n_faces <= 0) return;

        CQuake3BSP* bsp = modelRef->bsp;
        if (!bsp) return;

        // Group triangles by texture — vaudio assigns exactly one material
        // per mesh primitive, so each distinct texture used by this model
        // becomes its own primitive. Mirrors how the physics scene groups
        // collision shapes by texture (AddPhysicsBodyForEntityAndModel).
        std::unordered_map<std::string, std::vector<VAVector>> trianglesByTexture;

        for (int i = model.face; i < model.face + model.n_faces; i++)
        {
            if (i < 0 || i >= bsp->m_numOfFaces) continue;

            tBSPFace& face = bsp->m_pFaces[i];
            if (face.textureID < 0 || face.textureID >= MAX_TEXTURES) continue;

            std::string textureName(bsp->pTextures[face.textureID].strName);
            if (textureName.find("_cube") != std::string::npos) continue; // sky — doesn't occlude/reflect sound

            auto vertsIt = bsp->Rbuffers.v_faceVBOs.find(i);
            auto idxIt = bsp->Rbuffers.v_faceIDXs.find(i);
            if (vertsIt == bsp->Rbuffers.v_faceVBOs.end() || idxIt == bsp->Rbuffers.v_faceIDXs.end())
                continue;

            auto& verts = vertsIt->second;
            auto& idx = idxIt->second;

            auto& bucket = trianglesByTexture[textureName];
            bucket.reserve(bucket.size() + idx.size());

            // vaMeshPrimitiveCreate() takes a flat triangle soup (3 verts per
            // triangle, no index buffer) — unlike the physics/render paths,
            // so we expand indices here.
            for (uint32_t vi : idx)
            {
                if (vi >= verts.size()) continue;
                vec3 p = verts[vi].Position / MAP_SCALE; // raw BSP units -> engine units, same conversion physics/nav use
                bucket.push_back(ToVAVector(p));
            }
        }

        if (trianglesByTexture.empty()) return;

        VAMatrix transform = ComputeEntityTransform(modelRef);

        std::vector<VAMeshPrimitive*> primitives;
        std::vector<std::vector<VAVector>> localVerts;
        primitives.reserve(trianglesByTexture.size());
        localVerts.reserve(trianglesByTexture.size());

        for (auto& kv : trianglesByTexture)
        {
            const std::string& textureName = kv.first;
            std::vector<VAVector>& flatVerts = kv.second;

            if (flatVerts.size() < 3) continue;

            VAMaterialType material = ToVAMaterial(GetMaterialFromTexture(textureName));
            if (material == VAMaterialAir) continue; // "skip"/"trigger"/"clip"-style textures — not solid

            VAVector minB = flatVerts[0];
            VAVector maxB = flatVerts[0];
            for (const VAVector& v : flatVerts)
            {
                minB = vaVectorMin(minB, v);
                maxB = vaVectorMax(maxB, v);
            }

            VAMeshPrimitive* prim = vaMeshPrimitiveCreate(
                material, flatVerts.data(), (int)flatVerts.size(), minB, maxB, &transform);

            if (!prim) continue;

            AddPrimitiveToWorld(g_world, prim);
            primitives.push_back(prim);
            localVerts.push_back(std::move(flatVerts)); // kept only for SetDebugDrawGeometry()
        }

        if (primitives.empty()) return;

        EntityAudioGeometry geo;
        geo.entityId = entity->Id;
        geo.modelRef = modelRef;
        geo.primitives = std::move(primitives);
        geo.primitiveLocalVerts = std::move(localVerts);
        g_trackedEntities.push_back(std::move(geo));
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// BuildWorld
// ─────────────────────────────────────────────────────────────────────────────


void VisualisationCallback(VAVisualisationData* data, int count)
{
    for (auto i = 0; i < count; i++)
    {
        auto position = data[i].position;
        auto normal = data[i].normal;

        DebugDraw::Line(vec3(position.x, position.y, position.z), vec3(position.x + normal.x, position.y + normal.y, position.z + normal.z), 0.5f);

    }
}

void SpatialSoundManager::BuildWorld()
{
    // Tear down whatever the previous level built, if anything. This also
    // covers reloading the same level.
    Shutdown();

    if (!Level::Current) return;

    g_world = vaWorldCreate();

    // Rooms formed from currently-playing sounds' own reverb probes (see
    // CreateSoundEmitter()) get combined into at most this many clusters —
    // see SpatialReverb::direction / GetListenerReverb() below.
    vaWorldSetMaximumGroupedEAXCount(g_world, kMaxGroupedEAXCount);

    // BSP geometry is converted to engine units (raw BSP units / MAP_SCALE)
    // before being handed to vaudio (see BuildEntityGeometry above), so one
    // vaudio world-unit is one engine unit here. Tune this if the engine's
    // units don't correspond to ~1 meter.
    vaWorldSetMetersPerUnit(g_world, 1.0f);

    CQuake3BSP& bsp = Level::Current->BspData;
    if (!bsp.models.empty())
    {
        tBSPModel& worldModel = bsp.models[0]; // worldspawn — spans the whole level

        vec3 mn = vec3(worldModel.mins[0], worldModel.mins[1], worldModel.mins[2]) / MAP_SCALE;
        vec3 mx = vec3(worldModel.maxs[0], worldModel.maxs[1], worldModel.maxs[2]) / MAP_SCALE;

        const float margin = 64.0f; // keep emitters near the level's edge raytraced
        mn -= vec3(margin);
        mx += vec3(margin);


    }

    vaWorldSetPosition(g_world, ToVAVector(vec3(-500)));
    vaWorldSetSize(g_world, ToVAVector(vec3(2000)));

    Logger::Info("[SpatialSoundManager] world bounds: pos=(%f,%f,%f) size=(%f,%f,%f), bsp models=%zu",
        vaWorldGetPosition(g_world).x, vaWorldGetPosition(g_world).y, vaWorldGetPosition(g_world).z,
        vaWorldGetSize(g_world).x, vaWorldGetSize(g_world).y, vaWorldGetSize(g_world).z,
        bsp.models.size());

    // ── Listener ──────────────────────────────────────────────────────────
    g_listener = vaEmitterCreate();
    vaEmitterSetName(g_listener, "Listener");
    vaEmitterSetAffectsGroupedEax(g_listener, false); // listener shouldn't contribute to its own grouped reverb

    vaEmitterSetVisualisationRayCount(g_listener, 32);
    vaEmitterSetVisualisationBounceCount(g_listener, 3);
    vaEmitterSetVisualisationUpdateFrequency(g_listener, 500); // milliseconds
    vaEmitterSetVisualisationCallback(g_listener, VisualisationCallback);

    // Regular occlusion/permeation/reverb rays — used for positional sounds
    // via GetOcclusionFilter()/GetListenerReverb().
    vaEmitterSetOcclusionRayCount(g_listener, 32);
    vaEmitterSetOcclusionBounceCount(g_listener, 4);
    vaEmitterSetPermeationRayCount(g_listener, 32);
    vaEmitterSetPermeationBounceCount(g_listener, 3);
    vaEmitterSetReverbRayCount(g_listener, 128);
    vaEmitterSetReverbBounceCount(g_listener, 16);
    vaEmitterSetReverbEnergyCap(g_listener, 0.05f);
    vaEmitterSetMaxEchogramTime(g_listener, 5000);
    vaEmitterSetEchogramGranularity(g_listener, 50);

    // Directional reverb: lets us ask each Grouped EAX cluster (see
    // CreateSoundEmitter()) for its gain + pan direction relative to this
    // listener — that's what fills SpatialReverb::direction in
    // GetListenerReverb() below. Thresholds match the vaudio docs' example:
    // fully diffuse (pan to zero) once 50% of a group's rays return roughly
    // evenly, fully directional above 80%.
    vaEmitterSetHasRelativeReverb(g_listener, true);
    vaEmitterSetRelativeReverbInnerThreshold(g_listener, 0.5f);
    vaEmitterSetRelativeReverbOuterThreshold(g_listener, 0.8f);



    // Ambient occlusion/permeation rays — the "environment sound" feature.
    // Unlike the rays above, these are allowed to escape the level bounds;
    // GetOutsidePercent()/GetAmbientFilter() are only meaningful once these
    // are enabled (vaEmitterAmbientOcclusionEnabled()/
    // vaEmitterAmbientPermeationEnabled() need ray+bounce counts > 0).
    vaEmitterSetAmbientOcclusionRayCount(g_listener, 64);
    vaEmitterSetAmbientOcclusionBounceCount(g_listener, 6);
    vaEmitterSetAmbientOcclusionEnergyCap(g_listener, 0.5f); // tune from here

    vaEmitterSetAmbientPermeationRayCount(g_listener, 64);
    vaEmitterSetAmbientPermeationBounceCount(g_listener, 3);
    vaEmitterSetAmbientPermeationEnergyCap(g_listener, 0.5f); // tune from here

    vaWorldAddEmitter(g_world, g_listener);

    // ── World geometry, tied to the entity that owns it ─────────────────────
    // Brush entities each get exactly one BSPModelRef pushed into their
    // Drawables list by CQuake3BSP::LoadToLevel() — the same place that
    // builds the physics collision for that entity, using the entity's model
    // id. We reuse that existing relationship (rather than re-deriving
    // model-id -> entity-id ourselves from the "model" property) so this
    // system stays a pure, passive consumer of data that already exists.
    for (LevelObject* obj : Level::Current->LevelObjects)
    {
        Entity* entity = dynamic_cast<Entity*>(obj);
        if (!entity) continue;

        for (IDrawMesh* mesh : entity->Drawables)
        {
            BSPModelRef* modelRef = dynamic_cast<BSPModelRef*>(mesh);
            if (modelRef) BuildEntityGeometry(entity, modelRef);
        }
    }

    int primitiveCount = 0;
    for (auto& geo : g_trackedEntities) primitiveCount += (int)geo.primitives.size();

    Logger::Info("[SpatialSoundManager] Built vaudio world: %zu entities, %d primitives",
        g_trackedEntities.size(), primitiveCount);
}

// ─────────────────────────────────────────────────────────────────────────────
// Debug draw
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
    // Draws every tracked primitive's triangles as a wireframe. Reads the
    // transform back from vaudio (vaMeshPrimitiveGetTransform) rather than
    // recomputing it from the entity, so this always matches exactly what
    // vaudio is raytracing against right now — including on the frame a
    // moving brush's transform was just updated.
    void DrawDebugGeometry()
    {
        for (const EntityAudioGeometry& geo : g_trackedEntities)
        {
            for (size_t p = 0; p < geo.primitives.size(); ++p)
            {
                const VAMatrix* transform = vaMeshPrimitiveGetTransform(geo.primitives[p]);
                if (!transform) continue;

                const std::vector<VAVector>& verts = geo.primitiveLocalVerts[p];

                for (size_t t = 0; t + 3 <= verts.size(); t += 3)
                {
                    VAVector a = vaVectorMultiplyMatrix(verts[t + 0], transform);
                    VAVector b = vaVectorMultiplyMatrix(verts[t + 1], transform);
                    VAVector c = vaVectorMultiplyMatrix(verts[t + 2], transform);

                    vec3 va(a.x, a.y, a.z);
                    vec3 vb(b.x, b.y, b.z);
                    vec3 vc(c.x, c.y, c.z);

                    // Drawn every frame (Update() runs every frame while
                    // this is enabled), so a short duration is fine — it'll
                    // be refreshed before it would otherwise expire.
                    DebugDraw::Line(va, vb, 0.0f);
                    DebugDraw::Line(vb, vc, 0.0f);
                    DebugDraw::Line(vc, va, 0.0f);
                }
            }
        }
    }
}

void SpatialSoundManager::SetDebugDrawGeometry(bool enabled)
{
    g_debugDrawGeometry = enabled;
}

// ─────────────────────────────────────────────────────────────────────────────
// Update
// ─────────────────────────────────────────────────────────────────────────────

namespace
{
    std::vector<VAEmitter*> g_pendingAddTargets;
    std::vector<VAEmitter*> g_pendingRemoveTargets;
}

void SpatialSoundManager::Update()
{
    if (!g_world) return;

    vaWorldWait(g_world);

    vaEmitterSetPosition(g_listener, ToVAVector(Camera::position));

    for (size_t i = 0; i < g_trackedEntities.size(); )
    {
        EntityAudioGeometry& geo = g_trackedEntities[i];

        // No destruction hook exists (and none should be added — see the
        // class comment), so we poll instead: an entity that no longer
        // resolves by Id has been destroyed since our last tick.
        Entity* entity = Level::Current ? Level::Current->FindEntityWithId(geo.entityId) : nullptr;

        if (!entity)
        {
            for (VAMeshPrimitive* prim : geo.primitives)
            {
                RemovePrimitiveFromWorld(g_world, prim);
                vaMeshPrimitiveDestroy(prim);
            }

            g_trackedEntities[i] = std::move(g_trackedEntities.back());
            g_trackedEntities.pop_back();
            continue; // re-examine the element swapped into this slot
        }

        if (!entity->Static)
        {
            VAMatrix transform = ComputeEntityTransform(geo.modelRef);
            for (VAMeshPrimitive* prim : geo.primitives)
                vaMeshPrimitiveSetTransform(prim, &transform);
        }

        ++i;
    }

    if (!vaWorldGetThreadsRunning(g_world))
    {
        for (VAEmitter* e : g_pendingAddTargets)    vaEmitterAddTarget(g_listener, e);
        for (VAEmitter* e : g_pendingRemoveTargets)
        {
            vaEmitterRemoveTarget(g_listener, e);
            vaEmitterDestroy(e); // deferred from ReleaseSoundEmitter() - only safe now that
            // the listener has actually stopped targeting it
        }
        g_pendingAddTargets.clear();
        g_pendingRemoveTargets.clear();
    }

    vaWorldUpdate(g_world);

    Logger::Info("rays: %d", vaWorldGetRaysCastThisFrame(g_world));

    // After the loop above, so this reflects this frame's transforms for any
    // moving (non-static) brush geometry.
    if (g_debugDrawGeometry) DrawDebugGeometry();
}

// ─────────────────────────────────────────────────────────────────────────────
// Shutdown
// ─────────────────────────────────────────────────────────────────────────────

void SpatialSoundManager::Shutdown()
{
    if (!g_world)
    {
        g_trackedEntities.clear();
        g_listener = nullptr;
        return;
    }

    for (EntityAudioGeometry& geo : g_trackedEntities)
        for (VAMeshPrimitive* prim : geo.primitives)
        {
            RemovePrimitiveFromWorld(g_world, prim);
            vaMeshPrimitiveDestroy(prim);
        }
    g_trackedEntities.clear();

    if (g_listener)
    {
        vaWorldRemoveEmitter(g_world, g_listener);
        vaEmitterDestroy(g_listener);
        g_listener = nullptr;
    }

    // NOTE: a SpatialSoundEmitter created via CreateSoundEmitter() by a
    // still-playing sound is NOT tracked/owned here — SoundManager owns that
    // handle and is responsible for calling ReleaseSoundEmitter(). In
    // practice this is fine because entity-owned sounds are stopped/
    // destroyed during Level::CloseLevel() before BuildWorld() runs again. A
    // sound that deliberately survives a level transition (e.g. persistent
    // music) should release/recreate its emitter itself around the
    // transition, or it will leak one until it eventually stops.
    vaWorldDestroy(g_world);
    g_world = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// Sound-emitter API
// ─────────────────────────────────────────────────────────────────────────────



SpatialSoundEmitter* SpatialSoundManager::CreateSoundEmitter(const glm::vec3& position)
{
    if (!g_world || !g_listener) return nullptr;

    VAEmitter* e = vaEmitterCreate();
    vaEmitterSetPosition(e, ToVAVector(position));

    // Light reverb probe so this sound's local room contributes to the
    // world's Grouped EAX pool (see BuildWorld's vaEmitterSetHasRelativeReverb
    // and SpatialReverb::direction). Deliberately much lighter than the
    // listener's own reverb rays (128/16) since this runs once per
    // concurrently-playing 3D sound rather than once globally — a room with
    // no sound currently playing in it won't have a probe here at all, which
    // is the tradeoff of driving room detection off active sound emitters
    // rather than dedicated static probes.
    vaEmitterSetReverbRayCount(e, 24);
    vaEmitterSetReverbBounceCount(e, 8);
    vaEmitterSetReverbEnergyCap(e, 0.05f);
    vaEmitterSetAffectsGroupedEax(e, true);

    vaWorldAddEmitter(g_world, e);       // safe anytime - documented thread-safe

    g_pendingAddTargets.push_back(e);    // deferred instead of calling AddTarget here
    return ToOpaque(e);
}

void SpatialSoundManager::ReleaseSoundEmitter(SpatialSoundEmitter* emitter)
{
    if (!emitter) return;
    VAEmitter* e = FromOpaque(emitter);

    if (g_world) vaWorldRemoveEmitter(g_world, e); // safe anytime - documented thread-safe

    // If this emitter's AddTarget from CreateSoundEmitter() hasn't been
    // flushed yet (world threads have been busy since it was created), it
    // was never actually registered as a listener target. Cancel the
    // pending add and destroy it now — safe, since Update() will never see
    // this pointer. This is the common case for short-lived one-shot sounds
    // that spawn and finish faster than vaudio drains the pending queue.
    auto it = std::find(g_pendingAddTargets.begin(), g_pendingAddTargets.end(), e);
    if (it != g_pendingAddTargets.end())
    {
        g_pendingAddTargets.erase(it);
        vaEmitterDestroy(e);
        return;
    }

    // Otherwise the add is already flushed (or in flight) and vaudio may
    // still be treating this pointer as a live raytrace target. Destroying
    // it here would race Update()'s next AddTarget/RemoveTarget flush and
    // use-after-free inside vaudio. Defer the destroy until after Update()
    // has actually called RemoveTarget for it.
    g_pendingRemoveTargets.push_back(e);
}

void SpatialSoundManager::UpdateSoundEmitterPosition(SpatialSoundEmitter* emitter, const glm::vec3& position)
{
    if (emitter) vaEmitterSetPosition(FromOpaque(emitter), ToVAVector(position));
}

bool SpatialSoundManager::HasResultsFor(SpatialSoundEmitter* emitter)
{
    if (!g_listener || !emitter) return false;
    return vaEmitterHasRaytracedTarget(g_listener, FromOpaque(emitter));
}

SpatialLowPassFilter SpatialSoundManager::GetOcclusionFilter(SpatialSoundEmitter* emitter)
{
    if (!g_listener || !emitter) return SpatialLowPassFilter{};

    VAEmitter* e = FromOpaque(emitter);
    if (!vaEmitterHasRaytracedTarget(g_listener, e)) return SpatialLowPassFilter{};

    return ToSpatialFilter(vaEmitterGetTargetFilter(g_listener, e));
}

// ─────────────────────────────────────────────────────────────────────────────
// "Environment sound" — ambient occlusion/permeation rays that may escape
// the level bounds
// ─────────────────────────────────────────────────────────────────────────────

SpatialLowPassFilter SpatialSoundManager::GetAmbientFilter()
{
    if (!g_listener) return SpatialLowPassFilter{};
    return ToSpatialFilter(vaEmitterGetAmbientFilter(g_listener));
}

float SpatialSoundManager::GetOutsidePercent()
{
    if (!g_listener) return 0.0f;
    return vaEmitterGetOutsidePercent(g_listener);
}

// ─────────────────────────────────────────────────────────────────────────────
// Listener reverb
// ─────────────────────────────────────────────────────────────────────────────

bool SpatialSoundManager::IsReverbReady()
{
    return g_world && vaWorldGetReverbCalculated(g_world);
}

bool SpatialSoundManager::GetListenerReverb(SpatialReverb& outReverb)
{
    if (!g_listener || !IsReverbReady()) return false;

    const VAEAXReverb* eax = vaEmitterGetEAX(g_listener);
    if (!eax) return false;

    ToSpatialReverb(*eax, outReverb);

    // NOTE: the vaudio docs spell this function two different ways in
    // different sections ("vaEaxReverGetRelativeGain" vs.
    // "vaEaxReverbGetRelativeGain") — double-check the exact symbol names
    // against your installed vaudio.h; used here with the 'b' to match
    // VAEAXReverb's own naming and the docs' "Directional Reverb" section.
    outReverb.direction = ComputeRelativeReverbDirection();

    return true;
}

#else // DISABLE_VAUDIO

// ── Stub implementation: zero vaudio dependency, every call is a neutral no-op ──

void SpatialSoundManager::BuildWorld() {}
void SpatialSoundManager::Update() {}
void SpatialSoundManager::Shutdown() {}
void SpatialSoundManager::SetDebugDrawGeometry(bool) {}

SpatialSoundEmitter* SpatialSoundManager::CreateSoundEmitter(const glm::vec3&) { return nullptr; }
void SpatialSoundManager::ReleaseSoundEmitter(SpatialSoundEmitter*) {}
void SpatialSoundManager::UpdateSoundEmitterPosition(SpatialSoundEmitter*, const glm::vec3&) {}

bool SpatialSoundManager::HasResultsFor(SpatialSoundEmitter*) { return false; }
SpatialLowPassFilter SpatialSoundManager::GetOcclusionFilter(SpatialSoundEmitter*) { return SpatialLowPassFilter{}; }

SpatialLowPassFilter SpatialSoundManager::GetAmbientFilter() { return SpatialLowPassFilter{}; }
float SpatialSoundManager::GetOutsidePercent() { return 0.0f; }

bool SpatialSoundManager::IsReverbReady() { return false; }
bool SpatialSoundManager::GetListenerReverb(SpatialReverb&) { return false; }

#endif // DISABLE_VAUDIO