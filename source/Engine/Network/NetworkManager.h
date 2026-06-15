#pragma once

#include "NetPacket.h"
#include "INetworkTransport.h"

#include <cstdint>
#include <functional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declarations
class Level;
class NetworkedEntity;
class LevelObjectFactory;

// ---------------------------------------------------------------------------
// NetworkManager
//
// Static singleton.  Owns nothing; all lifetime is managed by the caller.
//
// Class identification
// --------------------
// There are no manually assigned class IDs in packet headers.  Instead,
// NetworkManager::Init reads LevelObjectFactory's registry (which is a
// std::map<string, …> and therefore always sorted the same way) and builds
// a uint16_t wire index for each registered type.  Index 0 is assigned to
// the alphabetically first name, 1 to the second, and so on.  Because both
// peers run identical code, the assignment is deterministic without any
// coordination.  The index is what travels in PT_SpawnEntity and
// PT_FullSnapshot.  Collision is structurally impossible: the map already
// enforces unique names.
//
// Sending model
// -------------
// Entity-update packets are accumulated every game frame and flushed once
// per network tick (default 20 Hz) as a single compressed bundle.
// Reliable packets (handshake, spawn, despawn, RPC) bypass the queue and
// are compressed and sent immediately.
// ---------------------------------------------------------------------------

class NetworkManager {
public:
    NetworkManager() = delete;

    // -----------------------------------------------------------------------
    // Lifecycle
    // -----------------------------------------------------------------------

    // Initialize.  Must be called before any level loads.
    //
    // Reads LevelObjectFactory::instance().GetRegistry() to build the stable
    // wire-index table.  The factory must be fully populated before this call.
    //
    // transport       — must remain alive for the session.
    // asServer        — true if this peer is the host (peerId = 0).
    // networkTickRate — entity-update flush rate in Hz (default 20).
    static void Init(INetworkTransport* transport, bool asServer,
                     float networkTickRate = 20.0f);

    // Tear down: flush queues, disconnect transport, reset all tables.
    // Safe to call even if Init was never called.
    static void Shutdown();

    // Called once per frame by the engine main loop.
    //
    // Phase 1 — Receive: poll transport, dispatch all arrived packets.
    // Phase 2 — Gather:  call PushNetworkUpdate on every owned entity,
    //                    enqueuing their state into s_updateQueue.
    // Phase 3 — Flush:   when the network tick interval has elapsed,
    //                    concatenate, compress, and send the update queue.
    //
    // No-op if isLoadingLevel is true.
    static void Tick(float dt);

    // -----------------------------------------------------------------------
    // State queries
    // -----------------------------------------------------------------------

    static bool    IsActive();
    static bool    IsServer();
    static bool    IsLoadingLevel();
    static uint8_t GetLocalPeerId();

    // -----------------------------------------------------------------------
    // Level load coordination  (called by Level, not by game entity code)
    // -----------------------------------------------------------------------

    // Mark start of a level load.  Resets the runtime ID sequence counter.
    // Must be called before Level::Load() begins adding entities.
    static void BeginLevelLoad(Level* level);

    // Called by Level::FinishLoading() after all load-phase entities are added.
    // Server → broadcasts PT_LevelLoadComplete; waits for PT_ClientReady.
    // Client → sends PT_ClientReady; waits for PT_LevelReady.
    static void OnLevelLoaded();

    // Called internally once all peers are confirmed ready.
    // Transitions out of loading state, fires OnNetworkSpawn, flushes queue.
    // Must not be called by game code.
    static void OnLevelReady();

    // -----------------------------------------------------------------------
    // Entity registration  (called by Level::AddEntity / RemoveEntity only)
    // -----------------------------------------------------------------------

    static void             Register  (NetworkedEntity* entity);
    static void             Unregister(uint64_t networkId);
    static NetworkedEntity* Find      (uint64_t networkId);

    // -----------------------------------------------------------------------
    // ID allocation
    // -----------------------------------------------------------------------

    // Allocates the next runtime ID in ownerId's namespace:
    //   (ownerId << 20) | localRuntimeSeq++
    static uint64_t AllocateRuntimeId(uint8_t ownerId);

    static uint64_t MakeLoadPhaseId(const std::string& entityId);

    // -----------------------------------------------------------------------
    // Broadcast helpers  (called by Level only)
    // -----------------------------------------------------------------------

    static void BroadcastSpawn  (NetworkedEntity* entity);
    static void BroadcastDespawn(uint64_t networkId);

    static void BroadcastOwnerChange(uint64_t networkId, uint8_t newOwner);

    // -----------------------------------------------------------------------
    // Per-entity update  (called by NetworkedEntity::PushNetworkUpdate only)
    // -----------------------------------------------------------------------

    // Appends a PT_EntityUpdate packet into s_updateQueue.
    // Not sent until the next network tick flush.
    static void EnqueueEntityUpdate(NetworkedEntity* entity);

    // -----------------------------------------------------------------------
    // RPC  (called by NetworkedEntity::SendRPC only)
    // -----------------------------------------------------------------------

    // Sends a reliable RPC immediately (never batched).
    // See RPC_HOWTO.md for the full explanation of authority and routing.
    static void SendRPC(uint64_t networkId, uint8_t rpcId,
                        NetPacket& args, RPCTarget target);

    // -----------------------------------------------------------------------
    // Server helpers
    // -----------------------------------------------------------------------

    // Spawn an entity on behalf of a remote client (e.g. PlayerCharacter).
    // Allocates an ID in the client's namespace and broadcasts PT_SpawnEntity.
    // Must only be called on the server.
    static void SpawnForPlayer(NetworkedEntity* entity, uint8_t targetPeerId);

    // Send the full world snapshot to a single late-joining client.
    // Must only be called on the server.
    static void SendFullSnapshotTo(uint8_t targetPeerId);

    // -----------------------------------------------------------------------
    // Transport callbacks  (wired internally; must not be called by game code)
    // -----------------------------------------------------------------------

    static void OnPacketReceived  (uint8_t senderId,
                                   const uint8_t* buffer, size_t length);
    static void OnPeerConnected   (uint8_t peerId);
    static void OnPeerDisconnected(uint8_t peerId);

    static float GetTickRate() { return s_networkTickRate; }

    static NetworkStat GetStat();

    static void DrawDebugUi();

private:
    // -----------------------------------------------------------------------
    // Static state
    // -----------------------------------------------------------------------

    static bool         s_isActive;
    static bool         s_isLoadingLevel;
    static bool         s_isServer;
    static uint8_t      s_localPeerId;
    static uint32_t     s_localRuntimeSeq;
    static uint16_t     s_outboundSeq;

    static uint32_t     s_loadTimeIdSeq;

    // Network tick accumulator
    static float        s_networkTickRate;   // Hz, set in Init
    static float        s_networkTickAccum;  // seconds accumulated since last flush
    static float        s_validationTickAccum;
    static float        kValidationInterval;

    static Level*                s_level;
    static INetworkTransport*    s_transport;

    // -----------------------------------------------------------------------
    // Class registry
    //
    // Built once in Init() from LevelObjectFactory::GetRegistry().
    // The factory's std::map is sorted, so the indices are stable across all
    // peers without any negotiation.
    //
    // s_nameToIndex : className  → uint16_t wire index (used when sending)
    // s_indexToName : wire index → className           (used when receiving)
    //
    // Factories for NetworkedEntity subclasses are fetched from
    // LevelObjectFactory at lookup time; we do not duplicate them here.
    // -----------------------------------------------------------------------
    static std::unordered_map<std::string, uint16_t> s_nameToIndex;
    static std::vector<std::string>                  s_indexToName;

    // networkId → entity
    static std::unordered_map<uint64_t, NetworkedEntity*> s_entities;

    // Server: peers that have not yet sent PT_ClientReady
    static std::set<uint8_t> s_pendingReadyClients;

    // Packets received before OnLevelReady() fires.
    // Stored as individual compressed NetPacket slices so FlushPreLiveQueue
    // can replay them through the normal OnPacketReceived path.
    struct QueuedPacket {
        uint8_t              senderId;
        std::vector<uint8_t> buffer; // one compressed NetPacket
    };
    static std::vector<QueuedPacket> s_preLiveQueue;

    // -----------------------------------------------------------------------
    // Outbound update queue
    //
    // Populated every frame by EnqueueEntityUpdate.  Flushed once per network
    // tick by FlushUpdateQueue.  Entries are grouped by routing target so all
    // updates for the same destination are concatenated into one compressed
    // blob — giving zlib more bytes to work with.
    //
    // Reliable packets (spawns, despawns, RPCs) are never queued here; they
    // are compressed and sent immediately via SendReliableNow.
    // -----------------------------------------------------------------------
    struct PendingUpdate {
        std::vector<uint8_t> bytes;        // finalized, uncompressed NetPacket bytes
        uint8_t              targetPeerId; // 255 = broadcast; 0–254 = specific peer
    };
    static std::vector<PendingUpdate> s_updateQueue;

    // -----------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------

    // Build s_nameToIndex / s_indexToName from LevelObjectFactory.
    static void BuildClassRegistry();

    // Resolve a wire index to a class name.  Returns "" if out of range.
    static const std::string& IndexToName(uint16_t index);

    // Resolve a class name to a wire index.  Asserts if not found.
    static uint16_t NameToIndex(const std::string& name);

    static void OnEntityListReceived(uint8_t senderId, NetPacket& packet);
    static void FlushPreLiveQueue();
    static void DispatchPacket(uint8_t senderId, NetPacket& packet);
    static void FlushUpdateQueue();

    // Compress bytes and send via transport immediately (reliable).
    // Routing: server → targetPeerId (255 = broadcast to all); client → server.
    static void SendReliableNow(const std::vector<uint8_t>& bytes,
                                uint8_t targetPeerId);

    // Compress and relay to all peers except one (reliable, server only).
    static void RelayReliableExcept(uint8_t excludePeerId,
                                    const std::vector<uint8_t>& bytes);

    // Compress and relay to all peers (reliable, server only).
    static void RelayReliableAll(const std::vector<uint8_t>& bytes);

    // Push finalized bytes into s_updateQueue for the next tick flush.
    static void EnqueueUpdate(std::vector<uint8_t> bytes, uint8_t targetPeerId);

    // Stamp senderId + outbound sequence number, finalize, return raw bytes.
    static std::vector<uint8_t> FinalizeOutbound(NetPacket& pkt);

    // Compress bytes and call the appropriate transport method.
    // Single primitive — all outgoing traffic passes through here.
    static void CompressAndSend(const uint8_t* data, size_t length,
                                uint8_t targetPeerId, bool reliable);

    // Returns true for packet types that bypass the pre-live queue and are
    // always sent reliably and immediately (never batched).
    static bool IsHandshakePacket(PacketType type);
};
