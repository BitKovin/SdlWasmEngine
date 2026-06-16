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
// Entity update pipeline (server → client)
// ----------------------------------------
// Frame N:  entities call PushNetworkUpdate → EnqueueEntityUpdate stores the
//           serialised payload in s_pendingEntityStates (map, latest wins).
// Tick:     FlushUpdateQueue runs the delta check per entity (FNV-1a hash vs
//           lastSentHash in s_entityUpdateCache).  Unchanged entities are
//           skipped; changed ones are concatenated and compressed as a single
//           blob per target.  lastSentPayload is also stored so corrections
//           can be resent without re-serialising.
//
// Reconciliation digest (client → server → corrections)
// -------------------------------------------------------
// Every kDigestInterval (0.5 s, 2 Hz) the client sends PT_EntityDigest:
//   uint16  entityCount
//   N ×  { uint64 networkId, uint32 stateHash }
// where stateHash is the hash of the last payload the client received for
// that entity (stored in s_entityReceivedHash; updated on every EntityUpdate
// and FullSnapshot receipt).
//
// The server ProcessEntityDigest() diffs the client's view against its own
// authoritative state (s_entities + s_entityUpdateCache) and sends targeted
// reliable corrections without touching the broadcast update queue:
//   • Client missing entity       → PT_SpawnEntity to that client
//   • Client hash ≠ server hash   → PT_EntityUpdate to that client
//   • Client has unknown entity   → PT_DespawnEntity to that client
// All corrections are batched into one CompressAndSend call per client.
//
// Late-join flow
// --------------
// Peers that connect while the level is already live are tracked in
// s_lateJoiners.  When their PT_ClientReady arrives the server sends them
// PT_LevelReady + PT_FullSnapshot (targeted) instead of broadcasting
// PT_LevelReady to everyone and calling OnLevelReady() server-side a second
// time.  The FullSnapshot populates the client's s_entityReceivedHash so
// the very first digest is already authoritative.
// ---------------------------------------------------------------------------

class NetworkManager {
public:
    NetworkManager() = delete;

    // -----------------------------------------------------------------------
    // Lifecycle
    // -----------------------------------------------------------------------

    static void Init(INetworkTransport* transport, bool asServer,
                     float networkTickRate = 20.0f);
    static void Shutdown();

    // One call per engine frame.
    //   Phase 1 — Receive:  poll transport, dispatch arrived packets.
    //   Phase 2 — Gather:   call PushNetworkUpdate on every owned entity.
    //   Phase 3 — Flush:    when the network tick interval has elapsed,
    //                       run delta check, compress, send.
    //   Phase 4 — Digest:   client sends PT_EntityDigest every 0.5 s.
    //   Phase 5 — Validate: server broadcasts FullSnapshot every 15 s.
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

    static void BeginLevelLoad(Level* level);
    static void OnLevelLoaded();
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

    static uint64_t AllocateRuntimeId(uint8_t ownerId);
    static uint64_t MakeLoadPhaseId(const std::string& entityId);

    // -----------------------------------------------------------------------
    // Broadcast helpers  (called by Level only)
    // -----------------------------------------------------------------------

    static void BroadcastSpawn      (NetworkedEntity* entity);
    static void BroadcastDespawn    (uint64_t networkId);
    static void BroadcastOwnerChange(uint64_t networkId, uint8_t newOwner);

    // -----------------------------------------------------------------------
    // Per-entity update  (called by NetworkedEntity::PushNetworkUpdate only)
    // -----------------------------------------------------------------------

    static void EnqueueEntityUpdate(NetworkedEntity* entity);

    // -----------------------------------------------------------------------
    // RPC
    // -----------------------------------------------------------------------

    static void SendRPC(uint64_t networkId, uint8_t rpcId,
                        NetPacket& args, RPCTarget target);

    // -----------------------------------------------------------------------
    // Server helpers
    // -----------------------------------------------------------------------

    static void SpawnForPlayer    (NetworkedEntity* entity, uint8_t targetPeerId);
    static void SendFullSnapshotTo(uint8_t targetPeerId);

    // -----------------------------------------------------------------------
    // Transport callbacks  (wired internally; must not be called by game code)
    // -----------------------------------------------------------------------

    static void OnPacketReceived  (uint8_t senderId,
                                   const uint8_t* buffer, size_t length);
    static void OnPeerConnected   (uint8_t peerId);
    static void OnPeerDisconnected(uint8_t peerId);

    static float       GetTickRate() { return s_networkTickRate; }
    static NetworkStat GetStat();
    static void        DrawDebugUi();

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

    static float        s_networkTickRate;
    static float        s_networkTickAccum;
    static float        s_validationTickAccum;
    static float        kValidationInterval;

    static Level*             s_level;
    static INetworkTransport* s_transport;

    // -----------------------------------------------------------------------
    // Class registry
    // -----------------------------------------------------------------------

    static std::unordered_map<std::string, uint16_t> s_nameToIndex;
    static std::vector<std::string>                  s_indexToName;

    // networkId → entity
    static std::unordered_map<uint64_t, NetworkedEntity*> s_entities;

    // Server: peers whose PT_ClientReady has not yet arrived (initial load)
    static std::set<uint8_t> s_pendingReadyClients;

    // Server: peers that connected while the level was already live.
    // Separated from s_pendingReadyClients so the initial-load "all ready?"
    // check ignores them.  Cleared when their PT_ClientReady arrives.
    static std::set<uint8_t> s_lateJoiners;

    struct QueuedPacket {
        uint8_t              senderId;
        std::vector<uint8_t> buffer;
    };
    static std::vector<QueuedPacket> s_preLiveQueue;

    // -----------------------------------------------------------------------
    // Delta-compression entity map
    //
    // Populated each frame by EnqueueEntityUpdate; keyed by networkId so
    // multiple writes per tick automatically keep the freshest state only.
    // Flushed and cleared by FlushUpdateQueue.
    // -----------------------------------------------------------------------

    struct PendingEntityState {
        std::vector<uint8_t> payloadBytes; // networkId + entity state, no header
        uint8_t              targetPeerId;
    };
    static std::unordered_map<uint64_t, PendingEntityState> s_pendingEntityStates;

    // -----------------------------------------------------------------------
    // Delta-compression cache
    //
    // One entry per entity, updated on every sent EntityUpdate.
    //
    // lastSentPayload is kept alongside lastSentHash so ProcessEntityDigest
    // can resend a correction without re-serialising the entity.
    // -----------------------------------------------------------------------

    struct EntityUpdateCache {
        uint32_t             lastSentHash     = 0;
        std::vector<uint8_t> lastSentPayload; // full EntityUpdate payload bytes
        float                timeSinceLastSent = 999.0f; // large → force first send
        bool                 everSent         = false;
    };
    static std::unordered_map<uint64_t, EntityUpdateCache> s_entityUpdateCache;

    // Resend unchanged entity state periodically as a reliability heartbeat.
    static constexpr float kIdleResendInterval = 3.0f; // seconds

    // -----------------------------------------------------------------------
    // Relay queue  (server only)
    //
    // Batches client-EntityUpdate relays so each unique sender produces one
    // compressed BroadcastExcept call per flush instead of one per entity.
    //
    // Also: when queueing a relay the server records the hash + payload in
    // s_entityUpdateCache so ProcessEntityDigest has a reference for
    // client-owned entities too.
    // -----------------------------------------------------------------------

    struct RelayUpdate {
        std::vector<uint8_t> bytes;
        uint8_t              excludePeerId;
    };
    static std::vector<RelayUpdate> s_relayQueue;

    // Legacy unreliable queue (non-entity unreliable updates, if any)
    struct PendingUpdate {
        std::vector<uint8_t> bytes;
        uint8_t              targetPeerId;
    };
    static std::vector<PendingUpdate> s_updateQueue;

    // -----------------------------------------------------------------------
    // Reconciliation digest
    //
    // Client-side: hash of the last payload received for each entity.
    //   Updated in DispatchPacket when an EntityUpdate or FullSnapshot arrives.
    //   Sent to the server in PT_EntityDigest every kDigestInterval seconds.
    //
    // Server-side: nothing extra — uses s_entityUpdateCache for its reference.
    // -----------------------------------------------------------------------

    static std::unordered_map<uint64_t, uint32_t> s_entityReceivedHash;

    static float               s_digestAccum;
    static constexpr float     kDigestInterval = 0.5f; // 2 Hz

    // -----------------------------------------------------------------------
    // Debug stats  (reset each flush, displayed in DrawDebugUi)
    // -----------------------------------------------------------------------

    struct DeltaStats {
        // Delta compression (per flush)
        uint32_t entitiesTotal   = 0;
        uint32_t entitiesSent    = 0;
        uint32_t entitiesSkipped = 0;
        // Reconciliation (per digest cycle, server-side)
        uint32_t digestMissing   = 0; // entities sent as SpawnEntity correction
        uint32_t digestStale     = 0; // entities sent as EntityUpdate correction
        uint32_t digestPhantom   = 0; // entities sent as DespawnEntity correction
    };
    static DeltaStats s_deltaStats;

    // -----------------------------------------------------------------------
    // Internal helpers
    // -----------------------------------------------------------------------

    static void BuildClassRegistry();
    static const std::string& IndexToName(uint16_t index);
    static uint16_t           NameToIndex(const std::string& name);

    static void OnEntityListReceived(uint8_t senderId, NetPacket& packet);
    static void FlushPreLiveQueue();
    static void DispatchPacket(uint8_t senderId, NetPacket& packet);
    static void FlushUpdateQueue();

    // Builds and sends PT_EntityDigest to the server (client only, 2 Hz).
    static void SendEntityDigest();

    // Diffs a client's digest against authoritative state and sends targeted
    // corrections (server only).  Corrections are reliable and bypass the
    // broadcast update queue entirely.
    static void ProcessEntityDigest(uint8_t clientPeerId, NetPacket& packet);

    static void SendReliableNow   (const std::vector<uint8_t>& bytes,
                                   uint8_t targetPeerId);
    static void RelayReliableExcept(uint8_t excludePeerId,
                                    const std::vector<uint8_t>& bytes);
    static void RelayReliableAll  (const std::vector<uint8_t>& bytes);

    static void EnqueueUpdate(std::vector<uint8_t> bytes, uint8_t targetPeerId);

    static std::vector<uint8_t> FinalizeOutbound(NetPacket& pkt);

    static void CompressAndSend(const uint8_t* data, size_t length,
                                uint8_t targetPeerId, bool reliable);

    static bool IsHandshakePacket(PacketType type);

    // FNV-1a 32-bit — fast dirty-check hash for delta compression + digest.
    static uint32_t HashPayload(const uint8_t* data, size_t size);
};
