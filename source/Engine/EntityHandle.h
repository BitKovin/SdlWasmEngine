// EntityHandle.h
#pragma once

#include <string>
#include <cstdint>
#include <Network/NetPacket.h>
#include <Entity.h>

class Entity;
class NetworkedEntity;
class Level;

// ---------------------------------------------------------------------------
// EntityHandle
//
// A network-stable reference to any entity.  Replaces ad-hoc (TargetType,
// clientId / networkId / entityId) tuples with a single reusable struct.
//
// Storage
// -------
//   isNetworked == true   →  identified by networkId (uint64_t)
//                             On resolve: scan LevelObjects for the
//                             NetworkedEntity whose networkId matches.
//                             If that entity has localProxy set, return
//                             localProxy instead (see proxy notes below).
//
//   isNetworked == false  →  identified by entityId (string)
//                             On resolve: scan LevelObjects by string Id.
//                             Only meaningful on the peer that has the entity
//                             (typically the server for non-replicated objects).
//
// Proxy entities
// --------------
// Some NetworkedEntity subclasses act as network stand-ins for a local
// entity that cannot be replicated directly.  Example: RemotePlayer is a
// NetworkedEntity that represents another peer's Player on this machine.
// On the owning peer, RemotePlayer sets localProxy = Player::Instance so
// that Resolve() returns the actual Player rather than the RemotePlayer
// shell.
//
//   localProxy is NOT serialised – it is set at runtime in Start() or
//   similar, and is only valid on the peer that owns the proxied entity.
//
// Ownership transfer
// ------------------
// GetOwnerPeerId() returns the peer id that should own an NPC targeting
// this handle:
//   • Networked handle → networkOwner of the found NetworkedEntity.
//   • Non-networked    → 0 (server).
// ---------------------------------------------------------------------------

struct EntityHandle
{
    bool        isNetworked = false;
    uint64_t    networkId   = 0;
    std::string entityId    = {};

    // ── Construction ──────────────────────────────────────────────────────

    static EntityHandle FromNetworked(uint64_t netId)
    {
        EntityHandle h;
        h.isNetworked = true;
        h.networkId   = netId;
        return h;
    }

    static EntityHandle FromEntity(const Entity* entity)
    {
        EntityHandle h;
        h.isNetworked = entity->networkProxyId > 0;
        h.networkId = entity->networkProxyId;
        h.entityId    = entity->Id;
        return h;
    }

    // ── Validity ──────────────────────────────────────────────────────────

    bool IsValid() const
    {
        return isNetworked ? (networkId != 0) : !entityId.empty();
    }

    void Clear() { *this = EntityHandle{}; }

    // ── Equality ──────────────────────────────────────────────────────────

    bool operator==(const EntityHandle& o) const
    {
        if (isNetworked != o.isNetworked) return false;
        return isNetworked ? (networkId == o.networkId) : (entityId == o.entityId);
    }
    bool operator!=(const EntityHandle& o) const { return !(*this == o); }

    // ── Resolve ───────────────────────────────────────────────────────────
    // Returns the entity this handle points to, or nullptr if not found.
    //
    // resolveProxy (default true):
    //   If the found NetworkedEntity has a non-null localProxy, return
    //   localProxy instead.  Pass false to get the NetworkedEntity shell
    //   itself regardless (e.g. for reading networkId / networkOwner).
    Entity* Resolve(bool resolveProxy = true);

    // ── Ownership ─────────────────────────────────────────────────────────
    // Returns the peer id that should own an NPC that is targeting this handle.
    // Returns 0 (server) when the entity is not found or not networked.
    uint8_t GetOwnerPeerId() const;

    // ── Wire serialization ────────────────────────────────────────────────

    void Write(NetPacket& packet) const
    {
        packet.WriteBool(isNetworked);
        if (isNetworked)
            packet.WriteUInt64(networkId);
        else
            packet.WriteString(entityId);
    }

    static EntityHandle Read(NetPacket& packet)
    {
        EntityHandle h;
        h.isNetworked = packet.ReadBool();
        if (h.isNetworked)
            h.networkId = packet.ReadUInt64();
        else
            h.entityId  = packet.ReadString();

        h.Resolve();

        return h;
    }
};
