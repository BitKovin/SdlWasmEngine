#include "NetworkManager.h"
#include "NetworkedEntity.h"
#include "LevelObjectFactory.h"
#include <Level.hpp>

#include <Helpers/ByteCompressor.h>

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <set>

// ---------------------------------------------------------------------------
// Static member definitions
// ---------------------------------------------------------------------------

bool         NetworkManager::s_isActive = false;
bool         NetworkManager::s_isLoadingLevel = false;
bool         NetworkManager::s_isServer = false;
uint8_t      NetworkManager::s_localPeerId = 0;
uint32_t     NetworkManager::s_localRuntimeSeq = 0;
uint16_t     NetworkManager::s_outboundSeq = 0;

uint32_t     NetworkManager::s_loadTimeIdSeq = 0;

float        NetworkManager::s_networkTickRate = 30.0f;
float        NetworkManager::s_networkTickAccum = 0.0f;
float        NetworkManager::s_validationTickAccum = 0.0f;
float        NetworkManager::kValidationInterval = 1.0f/5.0f;

Level* NetworkManager::s_level = nullptr;
INetworkTransport* NetworkManager::s_transport = nullptr;

std::unordered_map<std::string, uint16_t>  NetworkManager::s_nameToIndex;
std::vector<std::string>                   NetworkManager::s_indexToName;

std::unordered_map<uint64_t, NetworkedEntity*>  NetworkManager::s_entities;
std::set<uint8_t>                               NetworkManager::s_pendingReadyClients;
std::vector<NetworkManager::QueuedPacket>        NetworkManager::s_preLiveQueue;
std::vector<NetworkManager::PendingUpdate>       NetworkManager::s_updateQueue;

// ---------------------------------------------------------------------------
// Internal constants
// ---------------------------------------------------------------------------

namespace {

    constexpr uint8_t  BROADCAST_PEER_ID = 255;

    const std::string EMPTY_STRING;

} // namespace

// ---------------------------------------------------------------------------
// BuildClassRegistry
//
// Called once from Init().  Reads LevelObjectFactory::GetRegistry(), which
// returns a std::map<string, CreateEntityFn>.  std::map is always sorted by
// key, so iterating it produces an alphabetically ordered, deterministic
// sequence.  We assign wire index 0, 1, 2 … in that order.
//
// Every peer runs identical code and registers the same set of types, so the
// resulting index table is identical on all peers without any coordination.
// ---------------------------------------------------------------------------

void NetworkManager::BuildClassRegistry() {
    s_nameToIndex.clear();
    s_indexToName.clear();

    const auto& registry = LevelObjectFactory::instance().GetRegistry();

    // std::map iterates in ascending key order — guaranteed by the standard.
    uint16_t index = 0;
    for (const auto& [name, factory] : registry) {
        s_nameToIndex[name] = index;
        s_indexToName.push_back(name);
        ++index;
    }

    std::fprintf(stdout, "[NetworkManager] Built class registry: %zu types\n",
        s_indexToName.size());

#ifndef NDEBUG
    for (uint16_t i = 0; i < static_cast<uint16_t>(s_indexToName.size()); ++i) {
        std::fprintf(stdout, "  [%u] %s\n", i, s_indexToName[i].c_str());
    }
#endif
}

const std::string& NetworkManager::IndexToName(uint16_t index) {
    if (index >= static_cast<uint16_t>(s_indexToName.size())) {
        std::fprintf(stderr, "[NetworkManager] IndexToName: index %u out of range\n", index);
        return EMPTY_STRING;
    }
    return s_indexToName[index];
}

uint16_t NetworkManager::NameToIndex(const std::string& name) {
    auto it = s_nameToIndex.find(name);
    assert(it != s_nameToIndex.end() && "NameToIndex: class name not in registry");
    return it->second;
}

// ---------------------------------------------------------------------------
// Packet helpers
// ---------------------------------------------------------------------------

bool NetworkManager::IsHandshakePacket(PacketType type) {
    switch (type) {
    case PacketType::PeerIdAssign:
    case PacketType::LevelLoadComplete:
    case PacketType::ClientReady:
    case PacketType::LevelReady:
    case PacketType::FullSnapshot:
        return true;
    default:
        return false;
    }
}

std::vector<uint8_t> NetworkManager::FinalizeOutbound(NetPacket& pkt) {
    pkt.SetSenderId(s_localPeerId);
    pkt.SetSequenceNumber(s_outboundSeq++);
    return pkt.Finalize();
}

// ---------------------------------------------------------------------------
// CompressAndSend — the single transport primitive
// ---------------------------------------------------------------------------

void NetworkManager::CompressAndSend(const uint8_t* data, size_t length,
    uint8_t targetPeerId, bool reliable) {
    if (!s_transport) return;

    const std::vector<uint8_t> input(data, data + length);
    const std::vector<uint8_t> compressed = ByteCompressor::CompressData(input);

    if (targetPeerId == BROADCAST_PEER_ID) {
        s_transport->Broadcast(compressed.data(), compressed.size(), reliable);
    }
    else if (s_isServer) {
        s_transport->Send(targetPeerId, compressed.data(), compressed.size(), reliable);
    }
    else {
        // Clients always route through the server
        s_transport->Send(0, compressed.data(), compressed.size(), reliable);
    }
}

// ---------------------------------------------------------------------------
// Reliable send helpers — compress + send immediately, never queued
// ---------------------------------------------------------------------------

void NetworkManager::SendReliableNow(const std::vector<uint8_t>& bytes,
    uint8_t targetPeerId) {
    CompressAndSend(bytes.data(), bytes.size(), targetPeerId, /*reliable=*/true);
}

void NetworkManager::RelayReliableExcept(uint8_t excludePeerId,
    const std::vector<uint8_t>& bytes) {
    assert(s_isServer);
    const std::vector<uint8_t> compressed = ByteCompressor::CompressData(bytes);
    s_transport->BroadcastExcept(excludePeerId,
        compressed.data(), compressed.size(), /*reliable=*/true);
}

void NetworkManager::RelayReliableAll(const std::vector<uint8_t>& bytes) {
    assert(s_isServer);
    const std::vector<uint8_t> compressed = ByteCompressor::CompressData(bytes);
    s_transport->Broadcast(compressed.data(), compressed.size(), /*reliable=*/true);
}

// ---------------------------------------------------------------------------
// Unreliable update queue
// ---------------------------------------------------------------------------

void NetworkManager::EnqueueUpdate(std::vector<uint8_t> bytes, uint8_t targetPeerId) {
    s_updateQueue.push_back({ std::move(bytes), targetPeerId });
}

void NetworkManager::FlushUpdateQueue() {
    if (s_updateQueue.empty()) return;

    // 1. Take local ownership of the queue immediately. 
    // This perfectly protects against iterator invalidation if s_updateQueue 
    // is modified concurrently or via re-entrancy during the flush.
    std::vector<PendingUpdate> localQueue = std::move(s_updateQueue);
    s_updateQueue.clear();

    std::unordered_map<uint8_t, std::vector<uint8_t>> groups;

    // 2. Pre-calculate sizes and reserve capacity. 
    // This stops std::vector from repeatedly reallocating memory and copying 
    // itself as it grows, which prevents heap fragmentation and OOM crashes.
    std::unordered_map<uint8_t, size_t> groupSizes;
    for (const auto& entry : localQueue) {
        groupSizes[entry.targetPeerId] += entry.bytes.size();
    }
    for (const auto& [peerId, size] : groupSizes) {
        groups[peerId].reserve(size);
    }

    // 3. Move the data into the grouped buffers.
    for (auto& entry : localQueue) {
        auto& group = groups[entry.targetPeerId];
        // Using make_move_iterator to transfer ownership of the bytes safely
        group.insert(
            group.end(),
            std::make_move_iterator(entry.bytes.begin()),
            std::make_move_iterator(entry.bytes.end())
        );
    }

    // 4. Send the compressed blobs.
    for (auto& [targetPeerId, rawBytes] : groups) {
        CompressAndSend(rawBytes.data(), rawBytes.size(),
            targetPeerId, /*reliable=*/false);
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void NetworkManager::Init(INetworkTransport* transport, bool asServer,
    float networkTickRate) {
    assert(transport && "NetworkManager::Init: transport must not be null");
    assert(!s_isActive && "NetworkManager::Init called while already active");
    assert(networkTickRate > 0.0f && "networkTickRate must be positive");

    s_transport = transport;
    s_isServer = asServer;
    s_localPeerId = asServer ? 0 : 255; // 255 = unassigned; set by PT_PeerIdAssign
    s_isActive = true;
    s_isLoadingLevel = false;
    s_outboundSeq = 0;
    s_networkTickRate = networkTickRate;
    s_networkTickAccum = 0.0f;
    s_validationTickAccum = 0.0f;

    // Build wire-index table from LevelObjectFactory before any packets fly.
    BuildClassRegistry();

    // Wire transport callbacks.
    transport->onPeerConnected = [](uint8_t peerId) {
        NetworkManager::OnPeerConnected(peerId);
        };
    transport->onPeerDisconnected = [](uint8_t peerId) {
        NetworkManager::OnPeerDisconnected(peerId);
        };
    transport->onPacketReceived = [](uint8_t senderId,
        const uint8_t* data, size_t length) {
            NetworkManager::OnPacketReceived(senderId, data, length);
        };

    std::fprintf(stdout, "[NetworkManager] Initialized as %s, tick %.1f Hz\n",
        asServer ? "server" : "client", networkTickRate);
}

void NetworkManager::Shutdown() {
    if (!s_isActive) return;

    s_entities.clear();
    s_nameToIndex.clear();
    s_indexToName.clear();
    s_pendingReadyClients.clear();
    s_preLiveQueue.clear();
    s_updateQueue.clear();
    s_level = nullptr;
    s_isActive = false;
    s_isLoadingLevel = false;
    s_localRuntimeSeq = 0;
    s_networkTickAccum = 0.0f;
    s_validationTickAccum = 0.0f;

    if (s_transport) {
        s_transport->Disconnect();
        s_transport = nullptr;
    }

    std::fprintf(stdout, "[NetworkManager] Shut down\n");
}

// ---------------------------------------------------------------------------
// Tick — receive / gather / flush
// ---------------------------------------------------------------------------

void NetworkManager::Tick(float dt) {
    if (!s_isActive) return;

    // Phase 1: receive
    s_transport->Poll();

    if (s_isLoadingLevel) return;

    // Phase 2: gather — every owned entity enqueues its state
    for (auto& [id, entity] : s_entities) {
        if (entity->isOwned) {
            entity->PushNetworkUpdate(dt);
        }
    }

    // Phase 3: flush — send queued updates once per network tick
    s_networkTickAccum += dt;
    const float tickInterval = 1.0f / s_networkTickRate;

    if (s_networkTickAccum >= tickInterval) {
        s_networkTickAccum -= tickInterval;
        // Guard against spiral-of-death on very long frames
        if (s_networkTickAccum > tickInterval) {
            s_networkTickAccum = 0.0f;
        }
        FlushUpdateQueue();
    }

    // Phase 4: periodic entity-list validation (server only).
    // Broadcasts the full entity list so every client can reconcile any
    // spawns/despawns it missed (late join, dropped reliable, etc).
    if (s_isServer) {
        s_validationTickAccum += dt;
        if (s_validationTickAccum >= kValidationInterval) {
            s_validationTickAccum -= kValidationInterval;
            if (s_validationTickAccum > kValidationInterval) {
                s_validationTickAccum = 0.0f;
            }
            SendFullSnapshotTo(BROADCAST_PEER_ID);
        }
    }
}

// ---------------------------------------------------------------------------
// State queries
// ---------------------------------------------------------------------------

bool    NetworkManager::IsActive() { return s_isActive; }
bool    NetworkManager::IsServer() { return s_isServer; }
bool    NetworkManager::IsLoadingLevel() { return s_isLoadingLevel; }
uint8_t NetworkManager::GetLocalPeerId() { return s_localPeerId; }

// ---------------------------------------------------------------------------
// Level load coordination
// ---------------------------------------------------------------------------

void NetworkManager::BeginLevelLoad(Level* level) {
    assert(level && "BeginLevelLoad: level must not be null");

    s_level = level;
    s_isLoadingLevel = true;
    s_localRuntimeSeq = 0;
    s_loadTimeIdSeq = 0;
    s_networkTickAccum = 0.0f;
    s_preLiveQueue.clear();
    s_updateQueue.clear();

    std::fprintf(stdout, "[NetworkManager] Begin level load\n");
}

void NetworkManager::OnLevelLoaded() {
    assert(s_isActive);

    if (s_isServer) {
        NetPacket pkt(PacketType::LevelLoadComplete);
        auto bytes = FinalizeOutbound(pkt);
        const auto compressed = ByteCompressor::CompressData(bytes);
        s_transport->Broadcast(compressed.data(), compressed.size(), /*reliable=*/true);

        std::fprintf(stdout, "[NetworkManager] Sent PT_LevelLoadComplete, "
            "awaiting %zu client(s)\n", s_pendingReadyClients.size());

        if (s_pendingReadyClients.empty()) {
            OnLevelReady();
        }
    }
    else {
        NetPacket pkt(PacketType::ClientReady);
        SendReliableNow(FinalizeOutbound(pkt), /*targetPeerId=*/0);
        std::fprintf(stdout, "[NetworkManager] Sent PT_ClientReady\n");
    }
}

void NetworkManager::OnLevelReady() {
    s_isLoadingLevel = false;
    s_networkTickAccum = 0.0f;

    std::fprintf(stdout, "[NetworkManager] Level ready — OnNetworkSpawn on "
        "%zu entities\n", s_entities.size());

    for (auto& [id, entity] : s_entities) {
        entity->OnNetworkSpawn();
    }

    FlushPreLiveQueue();
}

void NetworkManager::FlushPreLiveQueue() {
    std::vector<QueuedPacket> queue = std::move(s_preLiveQueue);
    s_preLiveQueue.clear();

    std::fprintf(stdout, "[NetworkManager] Flushing %zu pre-live packets\n",
        queue.size());

    for (auto& qp : queue) {
        OnPacketReceived(qp.senderId, qp.buffer.data(), qp.buffer.size());
    }
}

// ---------------------------------------------------------------------------
// Entity registration
// ---------------------------------------------------------------------------

void NetworkManager::Register(NetworkedEntity* entity) {
    assert(entity);
    assert(entity->networkId != 0 && "Register: networkId must be non-zero");
    assert(s_entities.find(entity->networkId) == s_entities.end() &&
        "Register: duplicate networkId");

    s_entities[entity->networkId] = entity;
}

void NetworkManager::Unregister(uint64_t networkId) {
    s_entities.erase(networkId);
}

NetworkedEntity* NetworkManager::Find(uint64_t networkId) {
    auto it = s_entities.find(networkId);
    return (it != s_entities.end()) ? it->second : nullptr;
}

// ---------------------------------------------------------------------------
// ID allocation
// ---------------------------------------------------------------------------

uint64_t NetworkManager::AllocateRuntimeId(uint8_t ownerId) {
    assert(ownerId == s_localPeerId);

    // 32-bit id portion, offset so runtime IDs never collide with
    // level/load-time IDs (which stay below NETWORK_ID_RUNTIME_OFFSET).
    const uint32_t id = NETWORK_ID_RUNTIME_OFFSET + s_localRuntimeSeq++;

    // customData1 / customData2 are local-use placeholders for now.
    constexpr uint8_t  customData1 = 0;
    constexpr uint16_t customData2 = 0;

    const uint64_t networkId = PackNetworkId(id, ownerId, customData1, customData2);

    assert(networkId != 0);
    return networkId;
}

uint64_t NetworkManager::MakeLoadPhaseId(const std::string& entityId) 
{   

    s_loadTimeIdSeq++;
    
    return s_loadTimeIdSeq + 1;
}

// ---------------------------------------------------------------------------
// Broadcast helpers
// ---------------------------------------------------------------------------

void NetworkManager::BroadcastSpawn(NetworkedEntity* entity) {
    assert(entity && entity->networkId != 0);

    const uint16_t wireIndex = NameToIndex(entity->GetClassName());

    NetPacket pkt(PacketType::SpawnEntity);
    pkt.WriteUInt64(entity->networkId);
    pkt.WriteUInt16(wireIndex);           // class identified by wire index
    pkt.WriteUInt8(entity->networkOwner);
    entity->NetSerialize(pkt);

    auto bytes = FinalizeOutbound(pkt);

    if (s_isServer) {
        SendReliableNow(bytes, BROADCAST_PEER_ID);
    }
    else {
        SendReliableNow(bytes, /*server=*/0);
    }
}

void NetworkManager::BroadcastDespawn(uint64_t networkId) {
    NetPacket pkt(PacketType::DespawnEntity);
    pkt.WriteUInt64(networkId);

    auto bytes = FinalizeOutbound(pkt);

    if (s_isServer) {
        SendReliableNow(bytes, BROADCAST_PEER_ID);
    }
    else {
        SendReliableNow(bytes, /*server=*/0);
    }
}

void NetworkManager::BroadcastOwnerChange(uint64_t networkId, uint8_t newOwner) {
    NetPacket pkt(PacketType::OwnerChange);
    pkt.WriteUInt64(networkId);
    pkt.WriteUInt8(newOwner);

    auto bytes = FinalizeOutbound(pkt);

    if (s_isServer) {
        // Server broadcasts to every connected client immediately.
        SendReliableNow(bytes, BROADCAST_PEER_ID);
    }
    else {
        // Clients route through the server; it validates and relays.
        SendReliableNow(bytes, /*server=*/0);
    }
}

// ---------------------------------------------------------------------------
// Per-entity update (enqueue, not send)
// ---------------------------------------------------------------------------

void NetworkManager::EnqueueEntityUpdate(NetworkedEntity* entity) {
    assert(entity && entity->isOwned);

    NetPacket pkt(PacketType::EntityUpdate);
    pkt.WriteUInt64(entity->networkId);
    entity->NetSerialize(pkt);

    const uint8_t target = s_isServer ? BROADCAST_PEER_ID : uint8_t(0);
    EnqueueUpdate(FinalizeOutbound(pkt), target);
}

// ---------------------------------------------------------------------------
// RPC
// ---------------------------------------------------------------------------

void NetworkManager::SendRPC(uint64_t networkId, uint8_t rpcId,
    NetPacket& args, RPCTarget target) {
    NetPacket pkt(PacketType::RPC);
    pkt.WriteUInt64(networkId);
    pkt.WriteUInt8(rpcId);
    pkt.WriteUInt8(static_cast<uint8_t>(target));

    // Append the caller-built argument payload
    for (uint8_t b : args.GetPayloadBytes()) {
        pkt.WriteUInt8(b);
    }

    auto bytes = FinalizeOutbound(pkt);

    if (s_isServer) {
        // Apply locally first, then relay
        NetworkedEntity* entity = Find(networkId);
        if (entity) {
            NetPacket argsCopy = args.RewindedCopy();
            entity->OnRPC(rpcId, argsCopy);
        }
        switch (target) {
        case RPCTarget::All:
            RelayReliableAll(bytes);
            break;
        case RPCTarget::Others:
            // Server has no "sender" to exclude when it calls SendRPC
            // directly; treat Others as All.
            RelayReliableAll(bytes);
            break;
        case RPCTarget::Server:
            break; // server-only, already applied above
        }
    }
    else {
        // Client always sends to server; server applies and relays per target
        SendReliableNow(bytes, /*server=*/0);
    }
}

// ---------------------------------------------------------------------------
// Server helpers
// ---------------------------------------------------------------------------

void NetworkManager::SpawnForPlayer(NetworkedEntity* entity, uint8_t targetPeerId) {
    assert(s_isServer && "SpawnForPlayer must only be called on the server");
    assert(entity);

    // Set the owner to the target client
    entity->networkOwner = targetPeerId;

    // FIX: Let the Server allocate the ID using its OWN namespace and sequence.
    // This guarantees no collisions with client-generated local spawns.
    entity->networkId = AllocateRuntimeId(s_localPeerId);

    entity->isOwned = false;

    Register(entity);
    BroadcastSpawn(entity);
    entity->OnNetworkSpawn();
}

void NetworkManager::SendFullSnapshotTo(uint8_t targetPeerId) {
    assert(s_isServer && "SendFullSnapshotTo must only be called on the server");

    NetPacket pkt(PacketType::FullSnapshot);
    pkt.WriteUInt32(static_cast<uint32_t>(s_entities.size()));

    for (const auto& [id, entity] : s_entities) {
        pkt.WriteUInt64(entity->networkId);
        pkt.WriteUInt16(NameToIndex(entity->GetClassName())); // wire index
        pkt.WriteUInt8(entity->networkOwner);

        // [FIX]: Serialize to a temp packet to get the exact payload size
        NetPacket entityData(PacketType::EntityUpdate);
        entity->NetSerialize(entityData);

        const auto& bytes = entityData.GetPayloadBytes();

        // Write the payload size, followed by the raw bytes
        pkt.WriteUInt16(static_cast<uint16_t>(bytes.size()));
        for (uint8_t b : bytes) {
            pkt.WriteUInt8(b);
        }
    }

    SendReliableNow(FinalizeOutbound(pkt), targetPeerId);
}

// ---------------------------------------------------------------------------
// Transport callbacks
// ---------------------------------------------------------------------------

void NetworkManager::OnPeerConnected(uint8_t peerId) {
    std::fprintf(stdout, "[NetworkManager] Peer connected: %u\n", peerId);

    if (!s_isServer) return;

    // Always send peer ID first
    {
        NetPacket pkt(PacketType::PeerIdAssign);
        pkt.WriteUInt8(peerId);
        SendReliableNow(FinalizeOutbound(pkt), peerId);
    }

    if (s_isLoadingLevel) {
        s_pendingReadyClients.insert(peerId);
    }
    else {
        // Late joiner: send level name, then wait for ClientReady before snapshot
        {
            NetPacket pkt(PacketType::LevelInfo);
            pkt.WriteString(s_level ? s_level->filePath : "");
            SendReliableNow(FinalizeOutbound(pkt), peerId);
        }
        s_pendingReadyClients.insert(peerId);
    }
}

void NetworkManager::OnPeerDisconnected(uint8_t peerId) {
    std::fprintf(stdout, "[NetworkManager] Peer disconnected: %u\n", peerId);

    s_pendingReadyClients.erase(peerId);

    if (!s_isServer || !s_level) return;

    std::vector<uint64_t> toDestroy;
    std::vector<uint64_t> toMigrate;

    for (const auto& [id, entity] : s_entities) {
        if (entity->networkOwner != peerId) continue;

        // Entities that can migrate ownership survive under server control.
        // Everything else is destroyed so stale unowned objects don't linger.
        if (!entity->DestroyOnOwnerDisconnect && entity->CanMigrateOwner) {
            toMigrate.push_back(id);
        } else {
            toDestroy.push_back(id);
        }
    }

    for (uint64_t id : toDestroy) {
        BroadcastDespawn(id);
       
        if (NetworkedEntity* entity = Find(id)) {
            s_level->RemoveEntitySilent(entity);
        }
        Unregister(id);
    }

    // Transfer surviving entities to server ownership (peer 0).
    // BroadcastOwnerChange sends a reliable PacketType::OwnerChange to every
    // client so they update networkOwner / isOwned immediately.
    for (uint64_t id : toMigrate) {
        NetworkedEntity* entity = Find(id);
        if (!entity) continue;

        entity->networkOwner = 0;                    // server takes over
        entity->isOwned      = (s_localPeerId == 0); // always true on the server

        BroadcastOwnerChange(id, /*newOwner=*/0);

        std::fprintf(stdout,
            "[NetworkManager] Entity %u migrated to server after peer %u disconnected\n",
            id, peerId);
    }
}

// ---------------------------------------------------------------------------
// OnPacketReceived
//
// All inbound data from the transport arrives here as compressed bytes.
// A single compressed blob may contain multiple concatenated NetPackets
// (entity-update batches).  We decompress once then parse packets out of
// the flat buffer using the fixed-size header to find each boundary.
// ---------------------------------------------------------------------------

void NetworkManager::OnPacketReceived(uint8_t senderId,
    const uint8_t* buffer, size_t length) {
    const std::vector<uint8_t> compressed(buffer, buffer + length);
    std::vector<uint8_t> decompressed;

    try {
        decompressed = ByteCompressor::DecompressData(compressed);
    }
    catch (const std::exception& e) {
        std::fprintf(stderr, "[NetworkManager] Decompression failed from peer %u: %s\n",
            senderId, e.what());
        return;
    }

    size_t offset = 0;
    while (offset < decompressed.size()) {
        const uint8_t* ptr = decompressed.data() + offset;
        const size_t   remaining = decompressed.size() - offset;

        if (remaining < NetPacket::HEADER_SIZE) {
            std::fprintf(stderr, "[NetworkManager] Truncated header from peer %u "
                "(offset=%zu remaining=%zu)\n", senderId, offset, remaining);
            break;
        }

        // FIX: Read 32-bit payloadLength at bytes 4-7
        const uint32_t payloadLen =
            (static_cast<uint32_t>(ptr[4]) << 24) |
            (static_cast<uint32_t>(ptr[5]) << 16) |
            (static_cast<uint32_t>(ptr[6]) << 8) |
            static_cast<uint32_t>(ptr[7]);
        const size_t packetSize = NetPacket::HEADER_SIZE + payloadLen;

        if (remaining < packetSize) {
            std::fprintf(stderr, "[NetworkManager] Truncated packet from peer %u "
                "(need=%zu have=%zu)\n", senderId, packetSize, remaining);
            break;
        }

        NetPacket packet;
        if (!NetPacket::Parse(ptr, packetSize, packet)) {
            std::fprintf(stderr, "[NetworkManager] Bad checksum from peer %u "
                "at offset %zu — dropping rest of bundle\n", senderId, offset);
            break;
        }

        offset += packetSize;

        const PacketType type = packet.GetType();

        // Non-handshake packets that arrive before the level is ready are
        // queued as individual re-compressed slices and replayed later.
        if (!IsHandshakePacket(type) && s_isLoadingLevel) {
            const std::vector<uint8_t> slice(ptr, ptr + packetSize);
            QueuedPacket qp;
            qp.senderId = senderId;
            qp.buffer = ByteCompressor::CompressData(slice);
            s_preLiveQueue.push_back(std::move(qp));
            continue;
        }

        DispatchPacket(senderId, packet);
    }
}

// ---------------------------------------------------------------------------
// DispatchPacket
// ---------------------------------------------------------------------------

void NetworkManager::DispatchPacket(uint8_t senderId, NetPacket& packet) {
    const PacketType type = packet.GetType();

    switch (type) {

        // ── Handshake ────────────────────────────────────────────────────

    case PacketType::PeerIdAssign: {
        s_localPeerId = packet.ReadUInt8();
        std::fprintf(stdout, "[NetworkManager] Assigned peerId=%u\n", s_localPeerId);
        break;
    }

    case PacketType::LevelInfo: {
        const std::string levelName = packet.ReadString();
        std::fprintf(stdout, "[NetworkManager] Level info: '%s'\n", levelName.c_str());
        // Game code triggers the load; NetworkManager just logs here.
        // Extend by setting an onLevelInfoReceived callback if needed.
        break;
    }

    case PacketType::LevelLoadComplete: {
        NetPacket reply(PacketType::ClientReady);
        SendReliableNow(FinalizeOutbound(reply), /*server=*/0);
        std::fprintf(stdout, "[NetworkManager] Sent PT_ClientReady\n");
        break;
    }

    case PacketType::ClientReady: {
        assert(s_isServer);
        s_pendingReadyClients.erase(senderId);
        std::fprintf(stdout, "[NetworkManager] PT_ClientReady from peer %u "
            "(%zu remaining)\n", senderId, s_pendingReadyClients.size());

        if (s_pendingReadyClients.empty()) {
            NetPacket ready(PacketType::LevelReady);
            auto bytes = FinalizeOutbound(ready);
            const auto compressed = ByteCompressor::CompressData(bytes);
            s_transport->Broadcast(compressed.data(), compressed.size(), /*reliable=*/true);
            OnLevelReady();
        }
        break;
    }

    case PacketType::LevelReady: {
        std::fprintf(stdout, "[NetworkManager] Received PT_LevelReady\n");
        OnLevelReady();
        break;
    }

    case PacketType::FullSnapshot: {
        OnEntityListReceived(senderId, packet);
        break;
    }

                                 // ── Entity lifecycle ─────────────────────────────────────────────

    case PacketType::SpawnEntity: {
        const uint64_t networkId = packet.ReadUInt64();
        const uint16_t wireIndex = packet.ReadUInt16(); // was classId
        const uint8_t  networkOwner = packet.ReadUInt8();

        if (s_isServer) {
            NetPacket relay(PacketType::SpawnEntity);
            relay.WriteUInt64(networkId);
            relay.WriteUInt16(wireIndex);
            relay.WriteUInt8(networkOwner);
            packet.CopyRemainingTo(relay);
            RelayReliableExcept(senderId, FinalizeOutbound(relay));
        }

        if (Find(networkId)) break; // already known (e.g. our own spawn echoed back)

        const std::string& className = IndexToName(wireIndex);
        if (className.empty()) {
            std::fprintf(stderr, "[NetworkManager] SpawnEntity: unknown wire index %u\n",
                wireIndex);
            break;
        }

        // LevelObjectFactory constructs the entity by class name.
        // The factory returns Entity*; we downcast after verifying IsNetworked().
        Entity* raw = LevelObjectFactory::instance().create(className);
        if (!raw) {
            std::fprintf(stderr, "[NetworkManager] SpawnEntity: factory returned null "
                "for '%s'\n", className.c_str());
            break;
        }

        auto* entity = dynamic_cast<NetworkedEntity*>(raw);
        if (!entity) {
            std::fprintf(stderr, "[NetworkManager] SpawnEntity: '%s' is not a "
                "NetworkedEntity\n", className.c_str());
            delete raw;
            break;
        }

        entity->networkId = networkId;
        entity->networkOwner = networkOwner;
        entity->LoadAssetsIfNeeded();
        entity->Start();
        entity->NetDeserialize(packet);

        if (s_level) {
            s_level->AddEntity(entity);
        }
        break;
    }

    case PacketType::DespawnEntity: {
        const uint64_t networkId = packet.ReadUInt64();

        if (s_isServer) {
            NetPacket relay(PacketType::DespawnEntity);
            relay.WriteUInt64(networkId);
            RelayReliableExcept(senderId, FinalizeOutbound(relay));
        }

        NetworkedEntity* entity = Find(networkId);
        if (entity && s_level) {
            s_level->RemoveEntity(entity);
        }

        break;
    }

    case PacketType::EntityUpdate: {
        const uint64_t networkId = packet.ReadUInt64();

        if (s_isServer) {
            // Relay unreliably to all except sender, compressed, immediate.
            NetPacket relay(PacketType::EntityUpdate);
            relay.WriteUInt64(networkId);
            packet.CopyRemainingTo(relay);
            auto relayBytes = FinalizeOutbound(relay);
            const auto compressed = ByteCompressor::CompressData(relayBytes);
            s_transport->BroadcastExcept(senderId,
                compressed.data(), compressed.size(),
                /*reliable=*/false);
        }

        NetworkedEntity* entity = Find(networkId);
        if (entity && !entity->isOwned) {
            entity->NetDeserialize(packet);
        }
        break;
    }

                                 // ── RPC ──────────────────────────────────────────────────────────

    case PacketType::RPC: {
        const uint64_t  networkId = packet.ReadUInt64();
        const uint8_t   rpcId = packet.ReadUInt8();
        const RPCTarget target = static_cast<RPCTarget>(packet.ReadUInt8());

        if (s_isServer) {
            // Relay before applying so clients receive it even if OnRPC
            // removes the entity
            NetPacket relay(PacketType::RPC);
            relay.WriteUInt64(networkId);
            relay.WriteUInt8(rpcId);
            relay.WriteUInt8(static_cast<uint8_t>(target));
            packet.CopyRemainingTo(relay);
            auto relayBytes = FinalizeOutbound(relay);

            switch (target) {
            case RPCTarget::All:
                RelayReliableAll(relayBytes);
                break;
            case RPCTarget::Others:
                RelayReliableExcept(senderId, relayBytes);
                break;
            case RPCTarget::Server:
                break; // no relay
            }
        }

        NetworkedEntity* entity = Find(networkId);
        if (entity) {
            entity->OnRPC(rpcId, packet);
        }
        break;
    }

    // ── Ownership transfer ───────────────────────────────────────────────

    case PacketType::OwnerChange: {
        const uint64_t networkId = packet.ReadUInt64();
        const uint8_t  newOwner  = packet.ReadUInt8();

        if (s_isServer) 
        {
            // Relay to all clients before applying locally so they update even
            // if the entity's OnOwnerChanged callback were to remove it.
            NetPacket relay(PacketType::OwnerChange);
            relay.WriteUInt64(networkId);
            relay.WriteUInt8(newOwner);
            RelayReliableAll(FinalizeOutbound(relay));
        }

        NetworkedEntity* entity = Find(networkId);
        if (entity && entity->CanMigrateOwner) {
            entity->networkOwner = newOwner;
            entity->isOwned      = (newOwner == s_localPeerId);
        }
        break;
    }

    default:
        std::fprintf(stderr, "[NetworkManager] Unknown packet type %u from peer %u\n",
            static_cast<unsigned>(type), senderId);
        break;
    }
}

// ---------------------------------------------------------------------------
// Entity-list reconciliation (client-side, runs on every PT_FullSnapshot —
// both the initial late-join snapshot and every periodic validation tick)
// ---------------------------------------------------------------------------

void NetworkManager::OnEntityListReceived(uint8_t /*senderId*/, NetPacket& packet) {
    assert(!s_isServer);

    // Ensure loop counter matches the 32-bit type we fixed previously
    const uint32_t entityCount = packet.ReadUInt32();

    std::set<uint64_t> snapshotIds;
    std::vector<NetworkedEntity*> newlySpawned;

    for (uint32_t i = 0; i < entityCount; ++i) {
        const uint64_t networkId = packet.ReadUInt64();
        const uint16_t wireIndex = packet.ReadUInt16();
        const uint8_t  networkOwner = packet.ReadUInt8();

        // [FIX]: Read the exact size of this entity's payload
        const uint16_t payloadSize = packet.ReadUInt16();

        // [FIX]: Buffer the exact bytes so the main packet cursor advances safely
        NetPacket entityBuffer(PacketType::EntityUpdate);
        for (uint16_t p = 0; p < payloadSize; ++p) {
            entityBuffer.WriteUInt8(packet.ReadUInt8());
        }

        // Prepare the isolated buffer for reading
        NetPacket readPkt = entityBuffer.RewindedCopy();

        snapshotIds.insert(networkId);

        NetworkedEntity* existing = Find(networkId);
        if (existing) {
            existing->networkOwner = networkOwner;
            existing->isOwned = (networkOwner == s_localPeerId);
            // Feed it the isolated packet

            if (!existing->isOwned) {
                existing->NetDeserialize(readPkt);
            }
        }
        else {
            const std::string& className = IndexToName(wireIndex);
            if (className.empty()) {
                std::fprintf(stderr,
                    "[NetworkManager] EntityList: unknown wire index %u, skipping\n",
                    wireIndex);
                continue; // [FIX]: Changed from break to continue
            }

            Entity* raw = LevelObjectFactory::instance().create(className);
            if (!raw) continue; // [FIX]: Changed from break to continue

            auto* entity = dynamic_cast<NetworkedEntity*>(raw);
            if (!entity) { delete raw; continue; } // [FIX]: Changed from break to continue

            entity->networkId = networkId;
            entity->networkOwner = networkOwner;
            entity->isOwned = (networkOwner == s_localPeerId);

            if (!s_isLoadingLevel) {
                entity->LoadAssetsIfNeeded();
                entity->Start();
            }

            // Feed it the isolated packet
            entity->NetDeserialize(readPkt);

            if (s_level) {
                s_level->AddEntity(entity);
            }

            std::fprintf(stdout,
                "[NetworkManager] EntityList: spawning missed entity %u (%s)\n",
                networkId, className.c_str());

            if (!s_isLoadingLevel) {
                newlySpawned.push_back(entity);
            }
        }
    }

    // Remove local entities absent from the authoritative list — they were
    // despawned while we weren't listening (or we missed the despawn).
    // Skip entities we own but the server hasn't acknowledged yet: those are
    // pending local spawns, not stale entities.

    std::vector<NetworkedEntity*> toRemove;
    for (const auto& [id, entity] : s_entities) {
        if (snapshotIds.find(id) != snapshotIds.end()) continue;
        if (entity->isOwned) continue;
        toRemove.push_back(entity);
    }

    for (NetworkedEntity* entity : toRemove) {
        std::fprintf(stdout, "[NetworkManager] EntityList: removing stale entity %u\n",
            entity->networkId);
        Logger::Log(entity->Id);
        if (s_level) {
            if (s_isLoadingLevel) {
                s_level->RemoveEntitySilent(entity);
            }
            else {
                s_level->RemoveEntity(entity);
            }
        }
    }

    // Entities spawned outside the load phase need their spawn callback now;
    // load-phase spawns get it later from OnLevelReady().
    for (NetworkedEntity* entity : newlySpawned) {
        entity->OnNetworkSpawn();
    }
}