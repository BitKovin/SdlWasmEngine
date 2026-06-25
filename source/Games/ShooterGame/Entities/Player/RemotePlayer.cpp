#include "RemotePlayer.h"
#include "PlayerRepresentation.h"
#include "PlayerState.h"
#include "Player.hpp"
#include "LevelObjectFactory.h"
#include "Weapons/WeaponBase.h"
#include "Weapons/WeaponFirearm.h"
#include <Network/NetworkManager.h>
#include <AiPerception/AiPerceptionSystem.h>
#include <cassert>

#include "PlayerCloneArea/PlayerCloneAreaBase.h"

namespace
{
    // How far the rendered position is allowed to drift before we snap.
    constexpr float kSnapDistance = 2.5f;

    // Correction smoothing toward the extrapolated position.
    constexpr float kCorrectionInterpSpeed = 18.0f;

    // Prevent runaway prediction if packets stall.
    constexpr float kMaxPredictionTime = 0.25f;
}

// ─── Lifecycle ────────────────────────────────────────────────────────────────

RemotePlayer::RemotePlayer()
{
    ClassName = "remotePlayer";
    Tags = { "player" };
    DestroyOnOwnerDisconnect = true;
}

void RemotePlayer::LoadAssets()
{
    // Spawn the visual representation as a regular (non-networked) level entity.
    // It lives alongside this entity and is destroyed with it.
    representation = new PlayerRepresentation();
    Level::Current->AddEntity(representation, true);
    representation->LoadAssetsIfNeeded();
}

void RemotePlayer::Destroy()
{
    // Destroy the representation first so it is cleanly removed from the level
    // before RemotePlayer itself is taken out.
    if (representation)
    {
        representation->Destroy();
        representation = nullptr;
    }

    // The clone (if any) is a separate level entity and won't be cleaned up
    // by anything else, so it has to be torn down explicitly too.
    if (mirrorCloneRepresentation)
    {
        mirrorCloneRepresentation->Destroy();
        mirrorCloneRepresentation = nullptr;
    }

    Entity::Destroy();
}

// ─── Per-frame update ─────────────────────────────────────────────────────────

void RemotePlayer::LateUpdate()
{

    representation->OnlyShadows = isOwned;

    if (isOwned)
    {
        if (referencePlayer)
        {
            // Keep prediction fields in sync with the live player so
            // NetSerialize sends accurate data.
            targetPosition = referencePlayer->Position;
            targetRotation = referencePlayer->Rotation;
            playerHeight = referencePlayer->controller.isCrouched
                ? referencePlayer->controller.crouchHeight
                : referencePlayer->controller.height;
            predictedVelocity = referencePlayer->controller.GetVelocity();
            cameraRotation = referencePlayer->cameraRotation;

            weaponRIndex = GetWeaponIndexFromRef(referencePlayer->currentWeapon);
            weaponLIndex = GetWeaponIndexFromRef(referencePlayer->currentOffhandWeapon);

            weaponRHandlingType = referencePlayer->currentWeapon ? referencePlayer->currentWeapon->weaponHandlingType : 0;

            if (referencePlayer->currentWeapon)
            {
                WeaponFirearm* fw = dynamic_cast<WeaponFirearm*>(referencePlayer->currentWeapon);
                if (fw && fw->akimbo)
                    weaponRHandlingType = 2;
            }

            // Build state directly from the player pointer — model paths are
            // resolved without touching the weapon registry.
            if (representation)
            {
                PlayerState state = PlayerState::FromPlayerPtr(referencePlayer);
                representation->ApplyState(state);

                // Owned players don't render their own body, so the only
                // place their pose shows up is in a clone area (mirror).
                UpdatePlayerClone(state);
            }
        }

        timeSinceNetUpdate = 0.0f;
        Position = targetPosition;
        Rotation = targetRotation;

        if (observationTarget)
        {
            AiPerceptionSystem::RemoveTarget(observationTarget);
            observationTarget = nullptr;
        }
    }
    else
    {
        timeSinceNetUpdate += Time::DeltaTimeF;

        if (!observationTarget)
            observationTarget = AiPerceptionSystem::CreateTarget(Position, Id, { "player" });
    }

    // ── Dead-reckoning ────────────────────────────────────────────────────────
    // For the owned case timeSinceNetUpdate == 0, so this is a no-op and
    // Position stays equal to targetPosition set above.
    const float predictionTime = (timeSinceNetUpdate > kMaxPredictionTime)
        ? kMaxPredictionTime
        : timeSinceNetUpdate;

    const vec3  predictedPosition = targetPosition + (predictedVelocity * predictionTime);
    const float errorDistance = distance(Position, predictedPosition);

    if (errorDistance > kSnapDistance)
        Position = predictedPosition;
    else
        Position = MathHelper::Interp(Position, predictedPosition, Time::DeltaTimeF, kCorrectionInterpSpeed);

    Rotation = targetRotation;

    // ── Push final state to the representation (remote players only) ──────────
    if (!isOwned && representation)
    {
        PlayerState state;
        state.position = Position;
        state.rotation = Rotation;
        state.cameraRotation = cameraRotation;
        state.velocity = predictedVelocity;
        state.playerHeight = playerHeight;

        state.weaponRModelPath = weaponRModelPath;
        state.weaponLModelPath = weaponLModelPath;

        representation->Visible = true;
        representation->ApplyState(state);

        UpdatePlayerClone(state);
    }

    if (observationTarget)
        observationTarget->position = Position + vec3(0, 0.7f, 0);
}

void RemotePlayer::AsyncUpdate()
{
    // All visual / animation work lives in PlayerRepresentation::AsyncUpdate(),
    // which the level system calls automatically.
}

// ─── Network serialization ────────────────────────────────────────────────────

void RemotePlayer::NetSerialize(NetPacket& packet)
{
    packet.WriteVector3(targetPosition);
    packet.WriteVector3(targetRotation);
    packet.WriteFloat(playerHeight);
    packet.WriteVector3(predictedVelocity);
    packet.WriteVector3(cameraRotation);

    packet.WriteUInt16(weaponRIndex);
    packet.WriteUInt16(weaponLIndex);
    packet.WriteUInt8(weaponRHandlingType);
}

void RemotePlayer::NetDeserialize(NetPacket& packet)
{
    const vec3     newTargetPosition = packet.ReadVector3();
    const vec3     newTargetRotation = packet.ReadVector3();
    const float    newPlayerHeight = packet.ReadFloat();
    const vec3     incomingVelocity = packet.ReadVector3();

    lastNetPosition = targetPosition;
    targetPosition = newTargetPosition;
    targetRotation = newTargetRotation;
    playerHeight = newPlayerHeight;
    predictedVelocity = incomingVelocity;
    timeSinceNetUpdate = 0.0f;

    cameraRotation = packet.ReadVector3();

    const uint16_t newWeaponR = packet.ReadUInt16();
    const uint16_t newWeaponL = packet.ReadUInt16();
    const uint8_t     newHandling = packet.ReadUInt8();

    if (newWeaponR != weaponRIndex || newWeaponL != weaponLIndex)
    {
        weaponRIndex = newWeaponR;
        weaponLIndex = newWeaponL;
        RecalculateWeaponPaths(); // updates weaponRModelPath / weaponLModelPath
    }

    weaponRHandlingType = newHandling;
}

// ─── Weapon path resolution ───────────────────────────────────────────────────
// Paths are cached in RemotePlayer and pushed to PlayerRepresentation via
// PlayerState. The representation never touches the weapon registry.

void RemotePlayer::RecalculateWeaponPaths()
{
    auto resolvePath = [this](uint16_t index) -> std::string
        {
            if (index == UINT16_MAX)
                return {};

            const std::string weapClassname = GetClassNameFromId(index);
            if (weapClassname.empty())
                return {};

            auto weapEnt = LevelObjectFactory::instance().create(weapClassname);
            auto* weapPtr = dynamic_cast<Weapon*>(weapEnt);
            assert(weapPtr);

            auto* firearmPtr = dynamic_cast<WeaponFirearm*>(weapEnt);
            std::string path = firearmPtr ? firearmPtr->params.modelPathTp
                : weapPtr->thirdPersonModelPath;
            delete weapEnt;
            return path;
        };

    weaponRModelPath = resolvePath(weaponRIndex);
    weaponLModelPath = resolvePath(weaponLIndex);
}

void RemotePlayer::UpdatePlayerClone(const PlayerState& state)
{
    std::vector<Physics::HitResult> overlapingAreas = Physics::PointTrace(Position, BodyType::Area1);

    PlayerCloneAreaBase* cloneArea = nullptr;

    for (auto area : overlapingAreas)
    {
        PlayerCloneAreaBase* areaCast = dynamic_cast<PlayerCloneAreaBase*>(area.entity);

        if (areaCast)
        {
            cloneArea = areaCast;
        }
    }

    // Not standing in a clone area: drop the clone if we had one, and stop.
    if (!cloneArea)
    {
        if (mirrorCloneRepresentation)
        {
            mirrorCloneRepresentation->Destroy();
            mirrorCloneRepresentation = nullptr;
        }
        return;
    }

    // Lazily spawn the clone the first time it's needed. It's a regular,
    // independent PlayerRepresentation, same as the player's normal one.
    if (!mirrorCloneRepresentation)
    {
        mirrorCloneRepresentation = new PlayerRepresentation();
        Level::Current->AddEntity(mirrorCloneRepresentation, true);
        mirrorCloneRepresentation->LoadAssetsIfNeeded();
    }

    // Same pose as the regular representation (same PlayerState) — only the
    // transform differs, e.g. a mirror's reflection matrix.
    mirrorCloneRepresentation->transformModifier = cloneArea->GetTransformation();
    mirrorCloneRepresentation->Visible = true;
    mirrorCloneRepresentation->ApplyState(state);
}

// ─── Weapon registry cache ────────────────────────────────────────────────────
// Maps weapon class names ↔ factory registry indices. Used only during
// serialization; the representation layer works exclusively with model paths.

static std::map<std::string, uint16_t> weaponClassNameToIndexMap;
static std::map<uint16_t, std::string> weaponIndexToClassNameMap;

uint16_t RemotePlayer::GetWeaponIndexFromRef(Entity* ent)
{
    if (!ent) return UINT16_MAX;

    auto cached = weaponClassNameToIndexMap.find(ent->ClassName);
    if (cached != weaponClassNameToIndexMap.end())
        return cached->second;

    const auto& registry = LevelObjectFactory::instance().GetRegistry();
    auto it = registry.find(ent->ClassName);
    if (it == registry.end()) return UINT16_MAX;

    uint16_t index = static_cast<uint16_t>(std::distance(registry.begin(), it));
    weaponClassNameToIndexMap.emplace(ent->ClassName, index);
    weaponIndexToClassNameMap.emplace(index, ent->ClassName);
    return index;
}

std::string RemotePlayer::GetClassNameFromId(uint16_t id)
{
    if (id == UINT16_MAX) return {};

    auto cached = weaponIndexToClassNameMap.find(id);
    if (cached != weaponIndexToClassNameMap.end())
        return cached->second;

    const auto& registry = LevelObjectFactory::instance().GetRegistry();
    if (id >= registry.size()) return {};

    auto it = registry.begin();
    std::advance(it, id);

    const std::string& className = it->first;
    weaponIndexToClassNameMap.emplace(id, className);
    weaponClassNameToIndexMap.emplace(className, id);
    return className;
}

REGISTER_ENTITY(RemotePlayer, "remotePlayer")