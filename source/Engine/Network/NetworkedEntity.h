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

class NetworkedEntity : public Entity {
public:
    uint32_t networkId    = 0;
    uint8_t  networkOwner = 0;
    bool     isOwned      = false;

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

    // ── RPC ──────────────────────────────────────────────────────────────
    // Send a one-shot reliable event.  See RPC_HOWTO.md for full docs.
    void SendRPC(uint8_t rpcId, NetPacket& args,
                 RPCTarget target = RPCTarget::Server);

    virtual void OnRPC(uint8_t rpcId, NetPacket& args) {}

    void PushNetworkUpdate(float dt);
};
