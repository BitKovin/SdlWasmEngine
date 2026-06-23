#pragma once

#include <Network/NetworkedEntity.h>
#include <AiPerception/ObservationTarget.h>
#include "PlayerState.h"

class Player;
class PlayerRepresentation;

// Networked entity that represents a remote (or locally-owned) player.
//
// Responsibilities
//   - Receive and send player snapshots over the network.
//   - Run dead-reckoning / interpolation to smooth out packet jitter.
//   - Maintain the AI observation target for this player.
//   - Spawn and destroy a PlayerRepresentation that handles all visual output.
//
// Visual output is fully delegated to PlayerRepresentation. RemotePlayer
// builds a PlayerState every frame and pushes it to the representation.
// Additional representations (e.g. for mirrors) can be created externally
// by anyone who holds a pointer to this entity.
class RemotePlayer : public NetworkedEntity
{
public:
    RemotePlayer();
    ~RemotePlayer() = default;

    // Set by the local player system when this proxy represents the
    // locally-controlled player (isOwned == true).
    Player* referencePlayer = nullptr;

    // Primary visual representation. Spawned in LoadAssets(), destroyed in
    // Destroy(). Mirrors / portals may create their own representations
    // independently.
    PlayerRepresentation* representation = nullptr;

    std::shared_ptr<ObservationTarget> observationTarget;

    // ── Prediction state ──────────────────────────────────────────────────────
    vec3  targetPosition{};
    vec3  targetRotation{};
    vec3  cameraRotation{};
    vec3  lastNetPosition{};
    vec3  predictedVelocity{};

    float timeSinceNetUpdate = 0.0f;
    float playerHeight       = 0.0f;

    // ── Weapon state (indices for the wire, paths for the representation) ────
    uint16_t    weaponRIndex      = UINT16_MAX;
    uint16_t    weaponLIndex      = UINT16_MAX;
    bool        weaponRAkimbo     = false;
    std::string weaponRModelPath;
    std::string weaponLModelPath;

    // ── Virtuals ──────────────────────────────────────────────────────────────
    void Update()     override;
    void AsyncUpdate() override;
    void Destroy()    override;

    void NetSerialize(NetPacket& packet)   override;
    void NetDeserialize(NetPacket& packet) override;
    void LoadAssets() override;

    // Recomputes weaponRModelPath / weaponLModelPath from the current indices.
    // Called from NetDeserialize whenever weapon indices change.
    void RecalculateWeaponPaths();

    // Bidirectionally maps weapon class names ↔ registry indices.
    // Used only for serialization; the representation works with paths.
    uint16_t    GetWeaponIndexFromRef(Entity* ent);
    std::string GetClassNameFromId(uint16_t id);
};
