#pragma once

#include "Entity.h"
#include "NetPacket.h"

// ---------------------------------------------------------------------------
// NetworkedEntity
//
// Base class for all replicated entities.  Inherit from this instead of
// Entity directly.
//
// Class identification
// --------------------
// There are no manually assigned class IDs and no second registration step.
// LevelObjectFactory is the single source of truth for all entity types.
// NetworkManager::Init reads its registry (a std::map, always alphabetically
// sorted) and assigns a stable uint16_t wire index to each name.  The same
// sorted order on every peer means both sides agree on indices without any
// coordination.
//
// The only networking-specific requirement on a subclass is to override
// GetClassName() and return the exact name already registered with
// LevelObjectFactory:
//
//   // Bullet.h
//   const char* GetClassName() const override { return "Bullet"; }
//
// That is it.  No macro, no second factory call, no ID file.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// NetworkId packing (§ NetworkId)
//
// networkId is a 64-bit value with the following bit layout (MSB -> LSB):
//
//   [63..32]  id          (32 bits) - entity id. Runtime-spawned entities use
//                                      an id offset by NETWORK_ID_RUNTIME_OFFSET
//                                      to avoid colliding with level/load-time IDs.
//   [31..24]  clientId    (8 bits)  - peer/client id that spawned the entity
//   [23..16]  customData1 (8 bits)  - placeholder, local-use field
//   [15..0]   customData2 (16 bits) - placeholder, local-use field
//
// customData1 / customData2 are local variables for now and may be
// repurposed later.
// ---------------------------------------------------------------------------

constexpr uint32_t NETWORK_ID_RUNTIME_OFFSET = 0x80000000u; // load-time IDs stay below this

constexpr uint64_t NETWORK_ID_ID_BITS          = 32;
constexpr uint64_t NETWORK_ID_CLIENT_BITS      = 8;
constexpr uint64_t NETWORK_ID_CUSTOM1_BITS     = 8;
constexpr uint64_t NETWORK_ID_CUSTOM2_BITS     = 16;

constexpr uint64_t NETWORK_ID_ID_SHIFT      = NETWORK_ID_CLIENT_BITS + NETWORK_ID_CUSTOM1_BITS + NETWORK_ID_CUSTOM2_BITS; // 32
constexpr uint64_t NETWORK_ID_CLIENT_SHIFT  = NETWORK_ID_CUSTOM1_BITS + NETWORK_ID_CUSTOM2_BITS;                          // 24
constexpr uint64_t NETWORK_ID_CUSTOM1_SHIFT = NETWORK_ID_CUSTOM2_BITS;                                                    // 16
constexpr uint64_t NETWORK_ID_CUSTOM2_SHIFT = 0;

constexpr uint64_t NETWORK_ID_ID_MASK      = 0xFFFFFFFFull;
constexpr uint64_t NETWORK_ID_CLIENT_MASK  = 0xFFull;
constexpr uint64_t NETWORK_ID_CUSTOM1_MASK = 0xFFull;
constexpr uint64_t NETWORK_ID_CUSTOM2_MASK = 0xFFFFull;

inline uint64_t PackNetworkId(uint32_t id, uint8_t clientId,
                               uint8_t customData1, uint16_t customData2) {
    return (static_cast<uint64_t>(id)          << NETWORK_ID_ID_SHIFT)      |
           (static_cast<uint64_t>(clientId)    << NETWORK_ID_CLIENT_SHIFT)  |
           (static_cast<uint64_t>(customData1) << NETWORK_ID_CUSTOM1_SHIFT) |
           (static_cast<uint64_t>(customData2) << NETWORK_ID_CUSTOM2_SHIFT);
}

inline uint32_t NetworkIdGetId         (uint64_t netId) { return static_cast<uint32_t>((netId >> NETWORK_ID_ID_SHIFT)      & NETWORK_ID_ID_MASK); }
inline uint8_t  NetworkIdGetClientId   (uint64_t netId) { return static_cast<uint8_t> ((netId >> NETWORK_ID_CLIENT_SHIFT)  & NETWORK_ID_CLIENT_MASK); }
inline uint8_t  NetworkIdGetCustomData1(uint64_t netId) { return static_cast<uint8_t> ((netId >> NETWORK_ID_CUSTOM1_SHIFT) & NETWORK_ID_CUSTOM1_MASK); }
inline uint16_t NetworkIdGetCustomData2(uint64_t netId) { return static_cast<uint16_t>((netId >> NETWORK_ID_CUSTOM2_SHIFT) & NETWORK_ID_CUSTOM2_MASK); }

class NetworkedEntity : public Entity {
public:
    uint64_t networkId    = 0;
    uint8_t  networkOwner = 0;
    bool     isOwned      = false;

    Entity* LocalProxy = nullptr; 

    // ── Ownership policy ─────────────────────────────────────────────────
    // DestroyOnOwnerDisconnect (default: true)
    //   true  — entity is removed from the level when its owner disconnects.
    //           This is the safe default; stale unowned entities won't linger.
    //   false — entity is NOT destroyed on owner disconnect.  Pair with
    //           CanMigrateOwner=true so the server automatically inherits it
    //           and keeps it alive (e.g. dropped loot, persistent world objects).
    //
    // CanMigrateOwner (default: false)
    //   true  — ownership may be transferred at runtime via SetOwner /
    //           SetMeAsOwner.  NetworkManager will also re-home the entity to
    //           the server when its owner disconnects and
    //           DestroyOnOwnerDisconnect is false.
    //   false — ownership is fixed after spawn; SetOwner / SetMeAsOwner
    //           are no-ops, and the entity will always be destroyed on
    //           owner disconnect regardless of DestroyOnOwnerDisconnect.
    bool DestroyOnOwnerDisconnect = true;
    bool CanMigrateOwner          = false;

    virtual bool IsNetworked() const { return true; }

    // ── Serialization contract ───────────────────────────────────────────
    // Both overrides must read/write the same fields in the same order.
    virtual void NetSerialize(NetPacket& packet) {};
    virtual void NetDeserialize(NetPacket& packet) {};

    // ── Class name ───────────────────────────────────────────────────────
    // Return the exact name this type is registered under in LevelObjectFactory.
    // Implementation is one line in the subclass header or .cpp:
    //   const char* GetClassName() const override { return "Bullet"; }
    virtual const char* GetClassName() { return ClassName.c_str(); };

    // ── Lifecycle hooks ──────────────────────────────────────────────────
    virtual void OnNetworkSpawn()   {}
    virtual void OnNetworkDespawn() {}

    void Destroy() override;

    // ── Ownership transfer ───────────────────────────────────────────────
    // Both functions are no-ops when CanMigrateOwner is false.
    // The change is broadcast reliably to all peers via PacketType::OwnerChange.

    // Claim ownership for the local peer.
    void SetMeAsOwner();

    // Transfer ownership to an arbitrary peer (pass 0 for the server).
    void SetOwner(uint8_t peerId);

    // ── RPC ──────────────────────────────────────────────────────────────
    // Send a one-shot reliable event.  See RPC_HOWTO.md for full docs.
    void SendRPC(uint8_t rpcId, NetPacket& args,
                 RPCTarget target = RPCTarget::Server);

    virtual void OnRPC(uint8_t rpcId, NetPacket& args) {}

    void PushNetworkUpdate(float dt);
};
