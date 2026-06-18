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


float NetworkManager::kValidationInterval = 1 / 2.0f;

Level* NetworkManager::s_level = nullptr;
INetworkTransport* NetworkManager::s_transport = nullptr;

std::unordered_map<std::string, uint16_t> NetworkManager::s_nameToIndex;
std::vector<std::string>                  NetworkManager::s_indexToName;

std::unordered_map<uint64_t, NetworkedEntity*>      NetworkManager::s_entities;
std::set<uint8_t>                                   NetworkManager::s_pendingReadyClients;
std::set<uint8_t>                                   NetworkManager::s_lateJoiners;
std::vector<NetworkManager::QueuedPacket>            NetworkManager::s_preLiveQueue;

std::unordered_map<uint64_t, NetworkManager::PendingEntityState> NetworkManager::s_pendingEntityStates;
std::unordered_map<uint64_t, NetworkManager::EntityUpdateCache>  NetworkManager::s_entityUpdateCache;
std::vector<NetworkManager::RelayUpdate>                         NetworkManager::s_relayQueue;
std::vector<NetworkManager::PendingUpdate>                       NetworkManager::s_updateQueue;

std::unordered_map<uint64_t, uint32_t> NetworkManager::s_entityReceivedHash;

float NetworkManager::s_digestAccum = 0.0f;

std::unordered_map<uint64_t, float>                            NetworkManager::s_recentlyDespawned;
std::unordered_map<uint64_t, NetworkManager::UnconfirmedEntity> NetworkManager::s_unconfirmedEntities;

NetworkManager::DeltaStats NetworkManager::s_deltaStats = {};

// ---------------------------------------------------------------------------
// Internal constants and file-scope state
// ---------------------------------------------------------------------------

namespace {
    constexpr uint8_t  BROADCAST_PEER_ID = 255;
    const std::string  EMPTY_STRING;

    // ── FIX: Double-send before stale ────────────────────────────────────────
    //
    // Tracks how many consecutive sends of the same hash each entity has had.
    // An entity is only suppressed after 2 identical packets are sent, so
    // receivers always get one "confirmation copy" — this prevents a single
    // lost packet from leaving the remote in a diverged state for up to
    // kIdleResendInterval, which was the root cause of spawned entities
    // briefly appearing at their T=0 position before snapping to the correct
    // one on the first EntityUpdate.
    std::unordered_map<uint64_t, uint8_t> g_identicalSentCount;

    // ── FIX: Deferred spawn queue ─────────────────────────────────────────────
    //
    // BroadcastSpawn no longer sends immediately.  Instead it queues here, and
    // FlushUpdateQueue drains the queue on the next network tick, calling
    // NetSerialize at that point.  This ensures the spawn packet carries the
    // fully-settled post-physics state rather than the T=0 position, eliminating
    // the visible snap that occurred when the first EntityUpdate arrived with a
    // slightly different transform a frame later.
    struct PendingSpawn {
        uint64_t entityId;
        uint8_t  targetPeerId;
    };
    std::vector<PendingSpawn> g_pendingSpawns;

    // ── Maximum uncompressed bytes per unreliable EntityUpdate batch ──────────
    //
    // Every unreliable send goes through ByteCompressor then the transport as a
    // single UDP datagram.  Unreliable datagrams are NOT fragmented by ENet (or
    // most game transports), so the on-wire size must fit within the path MTU.
    //
    // Calculation (worst-case, incompressible payload):
    //   LZ4 compressBound(1100) = 1100 + (1100/255) + 16 = 1120 bytes compressed
    //   Transport overhead: IP(20) + UDP(8) + ENet unreliable header(~16) = 44 bytes
    //   Worst-case wire size: 1120 + 44 = 1164 bytes
    //
    //   MTU floors:  Ethernet = 1500, ENet default = 1400, IPv6 minimum = 1280.
    //   1164 < 1280 — safe on all three with comfortable margin.
    //
    // When the aggregated batch for a target exceeds this threshold, Phase 4 (and
    // Phase 3 for relay batches) splits it at NetPacket boundaries and issues
    // multiple sends.  Each send is independently compressed, so the receiver
    // processes them as separate bundles — no protocol changes needed.
    //
    // Tune upward if your transport supports larger unreliable datagrams, or if
    // you observe that entity-update throughput becomes the bottleneck at this
    // chunk size.  Do NOT raise it above compressBound⁻¹(MTU - overhead) for
    // your specific transport without verifying the transport can handle it.
    constexpr size_t kMaxUnreliableBatchBytes = 1100;

} // namespace

// ---------------------------------------------------------------------------
// HashPayload — FNV-1a 32-bit
// ~1 ns / byte; adequate collision resistance for dirty-checking.
// ---------------------------------------------------------------------------

uint32_t NetworkManager::HashPayload(const uint8_t* data, size_t size) {
    uint32_t hash = 2166136261u;
    for (size_t i = 0; i < size; ++i) {
        hash ^= static_cast<uint32_t>(data[i]);
        hash *= 16777619u;
    }
    return hash;
}

// ---------------------------------------------------------------------------
// BuildClassRegistry
// ---------------------------------------------------------------------------

void NetworkManager::BuildClassRegistry() {
    s_nameToIndex.clear();
    s_indexToName.clear();

    const auto& registry = LevelObjectFactory::instance().GetRegistry();
    uint16_t    index = 0;
    for (const auto& [name, factory] : registry) {
        s_nameToIndex[name] = index;
        s_indexToName.push_back(name);
        ++index;
    }

    std::fprintf(stdout, "[NetworkManager] Registry: %zu types\n",
        s_indexToName.size());
}

const std::string& NetworkManager::IndexToName(uint16_t index) {
    if (index >= static_cast<uint16_t>(s_indexToName.size()))
        return EMPTY_STRING;
    return s_indexToName[index];
}

uint16_t NetworkManager::NameToIndex(const std::string& name) {
    auto it = s_nameToIndex.find(name);
    assert(it != s_nameToIndex.end() && "NameToIndex: class not in registry");
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
// CompressAndSend — single transport primitive
// ---------------------------------------------------------------------------

void NetworkManager::CompressAndSend(const uint8_t* data, size_t length,
    uint8_t targetPeerId, bool reliable)
{
    if (!s_transport) return;
    const std::vector<uint8_t> compressed =
        ByteCompressor::CompressData(std::vector<uint8_t>(data, data + length));

    if (targetPeerId == BROADCAST_PEER_ID)
        s_transport->Broadcast(compressed.data(), compressed.size(), reliable);
    else if (s_isServer)
        s_transport->Send(targetPeerId, compressed.data(), compressed.size(), reliable);
    else
        s_transport->Send(0, compressed.data(), compressed.size(), reliable);
}

void NetworkManager::SendReliableNow(const std::vector<uint8_t>& bytes,
    uint8_t targetPeerId) {
    CompressAndSend(bytes.data(), bytes.size(), targetPeerId, /*reliable=*/true);
}

void NetworkManager::RelayReliableExcept(uint8_t excludePeerId,
    const std::vector<uint8_t>& bytes) {
    assert(s_isServer);
    const auto compressed = ByteCompressor::CompressData(bytes);
    s_transport->BroadcastExcept(excludePeerId,
        compressed.data(), compressed.size(), /*reliable=*/true);
}

void NetworkManager::RelayReliableAll(const std::vector<uint8_t>& bytes) {
    assert(s_isServer);
    const auto compressed = ByteCompressor::CompressData(bytes);
    s_transport->Broadcast(compressed.data(), compressed.size(), /*reliable=*/true);
}

void NetworkManager::EnqueueUpdate(std::vector<uint8_t> bytes, uint8_t targetPeerId) {
    s_updateQueue.push_back({ std::move(bytes), targetPeerId });
}

// ---------------------------------------------------------------------------
// EnqueueEntityUpdate
//
// Called every game frame per owned entity.  Stores the serialised payload
// in s_pendingEntityStates (map; latest call per tick wins automatically).
// The header is NOT stamped here — sequence numbers are consumed only for
// packets that actually survive the delta check in FlushUpdateQueue.
// ---------------------------------------------------------------------------

void NetworkManager::EnqueueEntityUpdate(NetworkedEntity* entity) {
    assert(entity && entity->isOwned);

    NetPacket pkt(PacketType::EntityUpdate);
    pkt.WriteUInt64(entity->networkId);
    entity->NetSerialize(pkt);

    PendingEntityState& pending = s_pendingEntityStates[entity->networkId];
    pending.payloadBytes = pkt.GetPayloadBytes(); // networkId + entity state
    pending.targetPeerId = s_isServer ? BROADCAST_PEER_ID : uint8_t(0);
}

// ---------------------------------------------------------------------------
// FlushUpdateQueue
//
// Phase 0 — Deferred spawns (NEW)
//   Drain g_pendingSpawns.  BroadcastSpawn queues here instead of sending
//   immediately so NetSerialize is called one network-tick later with the
//   post-physics settled state.  The delta cache is seeded from the same
//   serialization so the first regular EntityUpdate from FlushUpdateQueue is
//   treated as "second send of same state" rather than "first send" — this
//   dovetails with the double-send-before-stale change below.
//
// Phase 1 — Delta check:  skip entity if hash unchanged AND already sent
//   at least 2 identical packets.  (Was: skip after 1 identical packet.)
//   The second send acts as a confirmation copy so a single lost packet can
//   never leave the remote diverged for the full kIdleResendInterval window.
//
// Phase 2 — Stamp + group: for entities that pass, BuildFromPayload and
//   concatenate into per-target buffers.
//
// Phase 3 — Relay batch: group client relays by excludePeerId.
//
// Phase 4 — Send.
// ---------------------------------------------------------------------------

void NetworkManager::FlushUpdateQueue() {

    // ── Phase 0: Deferred spawns ─────────────────────────────────────────
    //
    // Drain before the hasWork check so spawns are never accidentally held
    // an extra tick just because there happen to be no entity-state changes.

    if (!g_pendingSpawns.empty()) {
        std::vector<PendingSpawn> spawns = std::move(g_pendingSpawns);

        for (const auto& ps : spawns) {
            NetworkedEntity* entity = Find(ps.entityId);
            if (!entity) continue; // destroyed between BroadcastSpawn and flush

            // Serialize fresh state once.  Reuse the cache-format payload
            // (networkId + state) for both the spawn packet and the delta-cache
            // seed so we only call NetSerialize once per entity.
            NetPacket cachePkt(PacketType::EntityUpdate);
            cachePkt.WriteUInt64(entity->networkId);
            entity->NetSerialize(cachePkt);
            const auto& cachePayload = cachePkt.GetPayloadBytes(); // networkId + state

            // Build the spawn packet: header fields first, then state bytes
            // (bytes 8+ of cachePayload, skipping the networkId prefix).
            NetPacket spawnPkt(PacketType::SpawnEntity);
            spawnPkt.WriteUInt64(entity->networkId);
            spawnPkt.WriteUInt16(NameToIndex(entity->GetClassName()));
            spawnPkt.WriteUInt8(entity->networkOwner);
            for (size_t i = 8; i < cachePayload.size(); ++i)
                spawnPkt.WriteUInt8(cachePayload[i]);

            SendReliableNow(FinalizeOutbound(spawnPkt), ps.targetPeerId);

            // Seed the delta cache for owned entities so the first regular
            // EntityUpdate from Phase 1 counts as the "second send" of this
            // state (iCount = 1 → send one more time → iCount = 2 → suppress).
            // Without this, the first EntityUpdate would reset iCount to 0 and
            // the entity would need to go through the full two-send cycle again
            // before it could be suppressed, doubling unnecessary traffic.
            if (entity->isOwned) {
                const uint32_t seedHash =
                    HashPayload(cachePayload.data(), cachePayload.size());
                EntityUpdateCache& cache = s_entityUpdateCache[entity->networkId];
                cache.lastSentHash = seedHash;
                cache.lastSentPayload = cachePayload;
                cache.everSent = true;
                cache.timeSinceLastSent = 0.0f;
                g_identicalSentCount[entity->networkId] = 1; // spawn = first send
            }
        }
    }

    const bool hasWork = !s_pendingEntityStates.empty()
        || !s_updateQueue.empty()
        || !s_relayQueue.empty();
    if (!hasWork) return;

    const float tickDt = 1.0f / s_networkTickRate;

    // Group key: high byte = targetPeerId, low byte = excludePeerId (255 = none).
    using GroupKey = uint16_t;
    auto makeKey = [](uint8_t target, uint8_t exclude) -> GroupKey {
        return static_cast<GroupKey>((uint16_t(target) << 8) | uint16_t(exclude));
        };
    std::unordered_map<GroupKey, std::vector<uint8_t>> groups;

    s_deltaStats.entitiesTotal = 0;
    s_deltaStats.entitiesSent = 0;
    s_deltaStats.entitiesSkipped = 0;

    // ── Phase 1: Owned entity delta check ────────────────────────────────

    for (auto& [entityId, pending] : s_pendingEntityStates) {
        ++s_deltaStats.entitiesTotal;

        const uint32_t hash = HashPayload(
            pending.payloadBytes.data(), pending.payloadBytes.size());

        EntityUpdateCache& cache = s_entityUpdateCache[entityId];
        cache.timeSinceLastSent += tickDt;

        const bool changed = !cache.everSent || (hash != cache.lastSentHash);
        const bool idleTimeout = (cache.timeSinceLastSent >= kIdleResendInterval);

        // FIX: suppress only after 2 identical packets have been sent, not 1.
        // The second packet is a confirmation copy: even if the first was lost
        // the remote will converge within one extra tick rather than waiting the
        // full kIdleResendInterval (~1 s) for the next forced resend.
        const uint8_t iCount = [&]() -> uint8_t {
            const auto it = g_identicalSentCount.find(entityId);
            return it != g_identicalSentCount.end() ? it->second : uint8_t(0);
            }();

        if (!changed && !idleTimeout && iCount >= 2) {
            ++s_deltaStats.entitiesSkipped;
            continue;
        }

        // Stamp header only now — no wasted sequence numbers for skipped packets.
        auto bytes = NetPacket::BuildFromPayload(
            PacketType::EntityUpdate, s_localPeerId, s_outboundSeq++,
            pending.payloadBytes);

        cache.lastSentHash = hash;
        cache.lastSentPayload = pending.payloadBytes;
        cache.timeSinceLastSent = 0.0f;
        cache.everSent = true;

        // Update the consecutive identical-send counter.
        //   changed or idleTimeout → this is a "first / renewed" send, reset to 1.
        //   !changed and !idleTimeout → this is a confirmation copy, increment.
        // Suppression fires when iCount reaches 2 (checked at the top of the loop
        // next tick), so the sequence is: send(1) → send(2) → suppress → ...
        if (changed || idleTimeout)
            g_identicalSentCount[entityId] = 1;
        else
            ++g_identicalSentCount[entityId];

        auto& group = groups[makeKey(pending.targetPeerId, 255)];
        group.insert(group.end(), bytes.begin(), bytes.end());

        ++s_deltaStats.entitiesSent;
    }
    s_pendingEntityStates.clear();

    // ── Phase 2: Legacy unreliable queue ─────────────────────────────────

    if (!s_updateQueue.empty()) {
        std::vector<PendingUpdate> localQueue = std::move(s_updateQueue);
        s_updateQueue.clear();
        for (auto& entry : localQueue) {
            auto& group = groups[makeKey(entry.targetPeerId, 255)];
            group.insert(group.end(), entry.bytes.begin(), entry.bytes.end());
        }
    }

    // ── Shared helper: split a concatenated NetPacket buffer at packet ───────
    //   boundaries and call sendFn for each chunk ≤ kMaxUnreliableBatchBytes.
    //
    // WHY THIS EXISTS:
    //   Unreliable UDP packets are not fragmented by the transport.  When many
    //   entities spawn simultaneously, Phase 1 above concatenates ALL of their
    //   EntityUpdate payloads into a single groups[] buffer.  If the compressed
    //   output of that buffer exceeds the transport's unreliable MTU (~1 400 B),
    //   one of two things happens depending on the transport / OS:
    //
    //     (a) The packet is silently dropped: the client never receives any of
    //         those EntityUpdates.
    //     (b) The OS sends the oversized UDP datagram via IP fragmentation.
    //         If a fragment is dropped in flight, or the receiver's SO_RCVBUF is
    //         smaller than the assembled datagram, recv() returns a TRUNCATED
    //         compressed bitstream.  The decompressor may:
    //           • throw on the truncated stream (caught, entire bundle discarded), or
    //           • successfully decompress the intact prefix, giving a shorter
    //             decompressed buffer.  The packet-parse loop then processes the
    //             first K inner packets correctly and breaks at the corrupted K+1th
    //             (truncated header / payloadLen garbage / bad checksum).
    //         Either way, entities K+1 … N receive no update.
    //
    //   In both cases, the server's delta-compression cache already recorded
    //   iCount = 2 for ALL entities (set during Phase 1 above, before the send).
    //   The server believes it delivered the packets.  Those entities are now
    //   suppressed for the next kIdleResendInterval seconds.
    //   The client waits until its kDigestInterval digest fires (~333 ms later)
    //   to receive corrections — producing the observed teleport snap.
    //
    // HOW IT WORKS:
    //   Each element of a groups[] buffer is a complete NetPacket produced by
    //   BuildFromPayload.  The on-wire layout is:
    //     [0]    packetType    (1 byte)
    //     [1]    senderId      (1 byte)
    //     [2-3]  sequenceNum   (uint16 big-endian)
    //     [4-7]  payloadLength (uint32 big-endian)  ← read here to find next boundary
    //     [8-9]  checksum      (uint16 CRC-16)
    //     [10 … 10+payloadLength-1]  payload
    //
    //   We walk the buffer reading payloadLength from bytes [cursor+4 … cursor+7],
    //   advance cursor by HEADER_SIZE + payloadLength, and flush a chunk whenever
    //   adding the NEXT packet would push the chunk over kMaxUnreliableBatchBytes.
    //   A single packet that is already larger than the threshold is sent alone
    //   (we cannot split within a packet).
    //
    // RECEIVER COMPATIBILITY:
    //   OnPacketReceived already loops over all inner packets in a decompressed
    //   bundle.  Splitting one large bundle into N smaller ones is transparent —
    //   each bundle is independently decompressed and dispatched identically.

    auto sendChunked = [&](const std::vector<uint8_t>& raw, auto sendFn) {
        size_t chunkStart = 0;
        size_t cursor     = 0;

        while (cursor < raw.size()) {
            // Guard: enough bytes for a complete header?
            if (cursor + NetPacket::HEADER_SIZE > raw.size()) break;

            // Read payloadLength from bytes [4..7] of the inner packet header.
            const uint32_t pl =
                (uint32_t(raw[cursor + 4]) << 24) | (uint32_t(raw[cursor + 5]) << 16) |
                (uint32_t(raw[cursor + 6]) <<  8) | uint32_t(raw[cursor + 7]);

            const size_t next = cursor + NetPacket::HEADER_SIZE + pl;
            if (next > raw.size()) break; // malformed inner packet (should never happen)

            // Flush the current chunk BEFORE advancing past this packet if
            // including it would exceed the limit — but only when the chunk is
            // non-empty (cursor > chunkStart).  If the chunk is empty, this
            // single packet already exceeds the threshold and must be sent alone.
            if (next - chunkStart > kMaxUnreliableBatchBytes && cursor > chunkStart) {
                sendFn(raw.data() + chunkStart, cursor - chunkStart);
                chunkStart = cursor;
            }

            cursor = next;
        }

        // Flush whatever remains (also covers the common fast-path where the
        // entire buffer fits in one chunk without ever entering the if above).
        if (chunkStart < cursor)
            sendFn(raw.data() + chunkStart, cursor - chunkStart);
    };

    // ── Phase 3: Relay queue (client EntityUpdates relayed by server) ─────

    if (!s_relayQueue.empty()) {
        std::unordered_map<uint8_t, std::vector<uint8_t>> relayGroups;
        {
            std::vector<RelayUpdate> localRelay = std::move(s_relayQueue);
            s_relayQueue.clear();
            for (auto& entry : localRelay) {
                auto& grp = relayGroups[entry.excludePeerId];
                grp.insert(grp.end(), entry.bytes.begin(), entry.bytes.end());
            }
        }
        for (auto& [excludeId, raw] : relayGroups) {
            sendChunked(raw, [&](const uint8_t* data, size_t len) {
                const auto compressed = ByteCompressor::CompressData(
                    std::vector<uint8_t>(data, data + len));
                s_transport->BroadcastExcept(excludeId,
                    compressed.data(), compressed.size(), /*reliable=*/false);
            });
        }
    }

    // ── Phase 4: Send owned-entity / legacy groups ────────────────────────
    // Uses sendChunked (defined above) so that when the aggregated buffer for
    // a target exceeds kMaxUnreliableBatchBytes the flush is split into multiple
    // unreliable sends rather than one oversized packet that the transport drops
    // or delivers truncated.

    for (auto& [key, raw] : groups) {
        if (raw.empty()) continue;
        const uint8_t targetPeerId = static_cast<uint8_t>((key >> 8) & 0xFF);
        sendChunked(raw, [&](const uint8_t* data, size_t len) {
            CompressAndSend(data, len, targetPeerId, /*reliable=*/false);
        });
    }
}

// ---------------------------------------------------------------------------
// SendEntityDigest  (client only, called every kDigestInterval seconds)
//
// Packet layout (before compression):
//   uint16  entityCount                        2 bytes
//   N ×   { uint64 networkId, uint32 hash }   12 bytes each
//
// For 200 entities: ~2.4 KB raw, ~1.5 KB compressed.
// At 2 Hz: ~3 KB/s — negligible compared to the entity update stream.
//
// The hash is the FNV-1a hash of the last EntityUpdate payload the client
// received for this entity (stored in s_entityReceivedHash).  Hash = 0 means
// the client knows the entity exists (it's in s_entities) but has never
// received a state update for it — treat as a missing-state case.
//
// Sent unreliably: a lost digest just waits for the next one 500 ms later.
// Server corrections in response are always sent reliably.
// ---------------------------------------------------------------------------

void NetworkManager::SendEntityDigest() {
    assert(!s_isServer);

    // Cap at uint16 max to respect the count field size.
    // Games with > 65535 networked entities need a redesign, not a larger field.
    const uint16_t count =
        static_cast<uint16_t>(std::min(s_entities.size(), size_t(0xFFFF)));

    NetPacket pkt(PacketType::EntityDigest);
    pkt.WriteUInt16(count);

    uint16_t written = 0;
    for (const auto& [id, entity] : s_entities) {
        if (written >= count) break;

        pkt.WriteUInt64(id);

        const auto it = s_entityReceivedHash.find(id);
        pkt.WriteUInt32(it != s_entityReceivedHash.end() ? it->second : 0u);

        ++written;
    }

    auto bytes = FinalizeOutbound(pkt);
    CompressAndSend(bytes.data(), bytes.size(), /*server(0)=*/0, /*reliable=*/false);
}

// ---------------------------------------------------------------------------
// ProcessEntityDigest  (server only)
//
// Diffs the client's claimed state against s_entities + s_entityUpdateCache.
// All corrections are batched into a single reliable CompressAndSend call
// to the requesting client — they do NOT enter the broadcast update queue.
//
// Stats (s_deltaStats.digest*) are reset here, at the top — they reflect the
// most recently processed digest cycle, not a session-cumulative total.
//
// Three cases:
//
//   A) Entity in s_entities, not in client digest
//      → Client missed a PT_SpawnEntity.
//      → Send PT_SpawnEntity with current state.
//
//   B) Entity in client digest, not in s_entities
//      → AMBIGUOUS — see the class-level comment "Bidirectional reconciliation
//        safety net". Only destroyed if s_recentlyDespawned confirms it was
//        really despawned. Otherwise tracked in s_unconfirmedEntities and,
//        if it persists past kSpawnResendRequestDelay, met with a
//        PT_SpawnRequest to the reporting peer — never destroyed on a guess.
//
//   C) Entity in both, but client hash ≠ server's lastSentHash
//      → Client missed an update (packet loss while entity was then suppressed).
//      → Send PT_EntityUpdate with lastSentPayload (no re-serialise needed).
//      → Skipped for entities this client owns — it is authoritative for those,
//        so the server should never tell it to "correct" its own state.
//
// The server resets the idle timers for entities it corrects so the broadcast
// heartbeat doesn't immediately duplicate the targeted correction.
// ---------------------------------------------------------------------------

void NetworkManager::ProcessEntityDigest(uint8_t clientPeerId, NetPacket& packet) {
    assert(s_isServer);

    // Reset to "this cycle only" — see comment above.
    s_deltaStats.digestMissing = 0;
    s_deltaStats.digestStale = 0;
    s_deltaStats.digestPhantom = 0;

    const uint16_t clientCount = packet.ReadUInt16();

    // Parse client's view: { networkId → stateHash }
    std::unordered_map<uint64_t, uint32_t> clientKnown;
    clientKnown.reserve(clientCount);
    for (uint16_t i = 0; i < clientCount; ++i) {
        const uint64_t id = packet.ReadUInt64();
        const uint32_t hash = packet.ReadUInt32();
        clientKnown[id] = hash;
    }

    std::vector<uint8_t> correctionBuffer;

    // ── Case A + C: iterate server entities ──────────────────────────────

    for (const auto& [id, entity] : s_entities) {
        // This ID is known to the server — any prior "unconfirmed" tracking
        // for it (from an earlier digest, before its SpawnEntity caught up)
        // is now resolved.  Clear it so we stop watching/requesting resends.
        s_unconfirmedEntities.erase(id);

        auto clientIt = clientKnown.find(id);

        if (clientIt == clientKnown.end()) {
            // Case A: client is missing this entity entirely.
            // Resend as a full SpawnEntity so the client can instantiate it.
            NetPacket spawnPkt(PacketType::SpawnEntity);
            spawnPkt.WriteUInt64(entity->networkId);
            spawnPkt.WriteUInt16(NameToIndex(entity->GetClassName()));
            spawnPkt.WriteUInt8(entity->networkOwner);
            entity->NetSerialize(spawnPkt);
            auto spawnBytes = FinalizeOutbound(spawnPkt);
            correctionBuffer.insert(correctionBuffer.end(),
                spawnBytes.begin(), spawnBytes.end());

            ++s_deltaStats.digestMissing;
        }
        else if (entity->networkOwner != clientPeerId) {
            // Case C: client has the entity but may have stale state.
            // (Skip entities this client owns — it's authoritative for those.)
            const auto cacheIt = s_entityUpdateCache.find(id);
            if (cacheIt != s_entityUpdateCache.end()
                && cacheIt->second.everSent
                && clientIt->second != cacheIt->second.lastSentHash) {

                // Resend using the cached payload — no re-serialisation.
                auto corrBytes = NetPacket::BuildFromPayload(
                    PacketType::EntityUpdate,
                    s_localPeerId,
                    s_outboundSeq++,
                    cacheIt->second.lastSentPayload);
                correctionBuffer.insert(correctionBuffer.end(),
                    corrBytes.begin(), corrBytes.end());

                // Reset idle timer so the next broadcast heartbeat doesn't
                // immediately duplicate this targeted correction.
                cacheIt->second.timeSinceLastSent = 0.0f;

                ++s_deltaStats.digestStale;
            }
        }
    }

    // ── Case B: client has entities the server doesn't ───────────────────
    //
    // Do NOT treat absence from s_entities as proof of staleness — see the
    // class-level comment.  Only a confirmed despawn justifies destroying
    // something on the client.  Anything else is tracked, not destroyed.

    for (const auto& [id, hash] : clientKnown) {
        if (s_entities.find(id) != s_entities.end()) continue; // handled above

        if (s_recentlyDespawned.find(id) != s_recentlyDespawned.end()) {
            // Confirmed: this ID really was despawned. Safe to tell the
            // client to remove it regardless of who used to own it.
            NetPacket despawnPkt(PacketType::DespawnEntity);
            despawnPkt.WriteUInt64(id);
            auto despawnBytes = FinalizeOutbound(despawnPkt);
            correctionBuffer.insert(correctionBuffer.end(),
                despawnBytes.begin(), despawnBytes.end());

            ++s_deltaStats.digestPhantom;
            continue;
        }

        // Unconfirmed — most likely a client-owned entity whose SpawnEntity
        // is still in flight or queued. Track it; do not destroy it.
        UnconfirmedEntity& unconfirmed = s_unconfirmedEntities[id];
        unconfirmed.reportedByPeerId = clientPeerId;

        if (unconfirmed.ageSinceFirstReported >= kSpawnResendRequestDelay
            && !unconfirmed.resendRequested) {
            // Persisted well past any plausible delivery delay for a normal
            // spawn/digest race — ask the owner to resend rather than
            // silently waiting (or, worse, destroying) forever.
            NetPacket reqPkt(PacketType::SpawnRequest);
            reqPkt.WriteUInt64(id);
            SendReliableNow(FinalizeOutbound(reqPkt), clientPeerId);
            unconfirmed.resendRequested = true;

            std::fprintf(stdout,
                "[NetworkManager] Entity %llu unconfirmed for %.1fs — "
                "requested resend from peer %u\n",
                static_cast<unsigned long long>(id),
                unconfirmed.ageSinceFirstReported, clientPeerId);
        }
    }

    // All corrections in one compressed reliable packet to this client.
    if (!correctionBuffer.empty()) {
        CompressAndSend(correctionBuffer.data(), correctionBuffer.size(),
            clientPeerId, /*reliable=*/true);
    }
}

// ---------------------------------------------------------------------------
// PruneReconciliationState  (server only, called once per Tick)
//
// Ages s_recentlyDespawned and s_unconfirmedEntities, erasing entries past
// their TTL.  Both maps are small and short-lived in the normal case (an
// entry resolves within one digest cycle or so); this just bounds the worst
// case (a peer that disconnects mid-reconciliation, or a genuinely corrupt
// ID that never resolves).
// ---------------------------------------------------------------------------

void NetworkManager::PruneReconciliationState(float dt) {
    for (auto it = s_recentlyDespawned.begin(); it != s_recentlyDespawned.end(); ) {
        it->second += dt;
        if (it->second >= kDespawnConfirmationWindow)
            it = s_recentlyDespawned.erase(it);
        else
            ++it;
    }

    for (auto it = s_unconfirmedEntities.begin(); it != s_unconfirmedEntities.end(); ) {
        it->second.ageSinceFirstReported += dt;
        if (it->second.ageSinceFirstReported >= kUnconfirmedEntityCeiling) {
            std::fprintf(stderr,
                "[NetworkManager] Giving up on unconfirmed entity %llu "
                "(reported by peer %u, never resolved)\n",
                static_cast<unsigned long long>(it->first),
                it->second.reportedByPeerId);
            it = s_unconfirmedEntities.erase(it);
        }
        else {
            ++it;
        }
    }
}
// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void NetworkManager::Init(INetworkTransport* transport, bool asServer,
    float networkTickRate)
{
    assert(transport && "NetworkManager::Init: transport must not be null");
    assert(!s_isActive && "NetworkManager::Init called while already active");
    assert(networkTickRate > 0.0f);

    s_transport = transport;
    s_isServer = asServer;
    s_localPeerId = asServer ? 0 : 255;
    s_isActive = true;
    s_isLoadingLevel = false;
    s_outboundSeq = 0;
    s_networkTickRate = networkTickRate;
    s_networkTickAccum = 0.0f;
    s_validationTickAccum = 0.0f;
    s_digestAccum = 0.0f;

    BuildClassRegistry();

    transport->onPeerConnected = [](uint8_t p) { NetworkManager::OnPeerConnected(p); };
    transport->onPeerDisconnected = [](uint8_t p) { NetworkManager::OnPeerDisconnected(p); };
    transport->onPacketReceived = [](uint8_t s, const uint8_t* d, size_t l) { NetworkManager::OnPacketReceived(s, d, l); };

    std::fprintf(stdout, "[NetworkManager] Init as %s, tick %.1f Hz, digest %.1f Hz\n",
        asServer ? "server" : "client", networkTickRate, 1.0f / kDigestInterval);
}

void NetworkManager::Shutdown() {
    if (!s_isActive) return;

    s_entities.clear();
    s_nameToIndex.clear();
    s_indexToName.clear();
    s_pendingReadyClients.clear();
    s_lateJoiners.clear();
    s_preLiveQueue.clear();

    s_pendingEntityStates.clear();
    s_entityUpdateCache.clear();
    s_relayQueue.clear();
    s_updateQueue.clear();

    s_entityReceivedHash.clear();
    s_digestAccum = 0.0f;

    s_recentlyDespawned.clear();
    s_unconfirmedEntities.clear();

    // Clear file-scope implementation caches.
    g_identicalSentCount.clear();
    g_pendingSpawns.clear();

    s_level = nullptr;
    s_isActive = false;
    s_isLoadingLevel = false;
    s_localRuntimeSeq = 0;
    s_networkTickAccum = 0.0f;
    s_validationTickAccum = 0.0f;
    s_deltaStats = {};

    if (s_transport) {
        s_transport->Disconnect();
        s_transport = nullptr;
    }

    std::fprintf(stdout, "[NetworkManager] Shut down\n");
}

// ---------------------------------------------------------------------------
// Tick
//
// Phase 1 — Receive
// Phase 2 — Gather (owned entities)
// Phase 3 — Flush  (network tick rate, 20–30 Hz)
// Phase 4 — Digest (client only, 3 Hz)
// Phase 5 — Validation snapshot + client-state reapply (server only, 2 Hz)
// ---------------------------------------------------------------------------

void NetworkManager::Tick(float dt) {
    if (!s_isActive) return;

    // Phase 1: receive
    s_transport->Poll();

    if (s_isLoadingLevel) return;

    // Phase 2: gather — every owned entity enqueues its current state.
    // Multiple writes per tick for the same entity are harmless (map key
    // ensures latest state wins).
    for (auto& [id, entity] : s_entities) {
        if (entity->isOwned)
            entity->PushNetworkUpdate(dt);
    }

    // Phase 3: flush
    s_networkTickAccum += dt;
    const float tickInterval = 1.0f / s_networkTickRate;
    if (s_networkTickAccum >= tickInterval) {
        s_networkTickAccum -= tickInterval;
        if (s_networkTickAccum > tickInterval)
            s_networkTickAccum = 0.0f;
        FlushUpdateQueue();
    }

    // Phase 4: digest (client only)
    // The client sends a compact (id, hash) list every 0.5 s so the server
    // can spot and correct any divergence caused by packet loss + suppression.
    if (!s_isServer) {
        s_digestAccum += dt;
        if (s_digestAccum >= kDigestInterval) {
            s_digestAccum -= kDigestInterval;
            if (s_digestAccum > kDigestInterval)
                s_digestAccum = 0.0f;
            SendEntityDigest();
        }
    }

    // Phase 5: validation snapshot + server-side client-state reapply (server only)
    if (s_isServer) {
        s_validationTickAccum += dt;
        if (s_validationTickAccum >= kValidationInterval) {
            s_validationTickAccum -= kValidationInterval;
            if (s_validationTickAccum > kValidationInterval)
                s_validationTickAccum = 0.0f;

            // Send the authoritative snapshot so clients can correct any
            // divergence caused by locally moving a server-owned entity.
            // kValidationInterval = 0.5 s (2 Hz), so clients snap back within
            // one validation period at most.
            SendFullSnapshotTo(BROADCAST_PEER_ID);

            // FIX: symmetric server-side snap-back for client-owned entities.
            //
            // The FullSnapshot above fixes CLIENT divergence for server-owned
            // entities.  But when the server accidentally moves a client-owned
            // entity locally (e.g. a server physics body wrongly affecting it),
            // the correction only came from the client's next EntityUpdate —
            // which was suppressed by delta compression until kIdleResendInterval
            // (~1 s), causing the very noticeable delay reported.
            //
            // Fix: every kValidationInterval, reapply the last payload the
            // client actually sent us (stored verbatim in
            // s_entityUpdateCache[id].lastSentPayload by the EntityUpdate
            // dispatch handler) to the server's local entity representation.
            // This bounds the drift window to kValidationInterval (0.2 s),
            // matching the snap-back latency clients experience for server-owned
            // entities — making both directions symmetric.
            //
            // lastSentPayload layout: [ networkId : 8 bytes ][ state bytes : N ]
            // We skip the networkId prefix and feed the state bytes into a
            // temporary packet for NetDeserialize, exactly as the live
            // EntityUpdate path does after its ReadUInt64() call.
            for (auto& [id, entity] : s_entities) {
                if (entity->isOwned) continue; // server-owned — skip

                const auto cit = s_entityUpdateCache.find(id);
                if (cit == s_entityUpdateCache.end() || !cit->second.everSent)
                    continue; // no client state received yet — nothing to reapply

                const auto& payload = cit->second.lastSentPayload;
                if (payload.size() <= 8) continue; // no state bytes after networkId

                NetPacket statePkt(PacketType::EntityUpdate);
                for (size_t i = 8; i < payload.size(); ++i)
                    statePkt.WriteUInt8(payload[i]);

                NetPacket readPkt = statePkt.RewindedCopy();
                entity->NetDeserialize(readPkt);
            }
        }

        // Age out confirmed-despawn / unconfirmed-entity tracking used by
        // ProcessEntityDigest's Case B.
        PruneReconciliationState(dt);
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
    assert(level);

    s_level = level;
    s_isLoadingLevel = true;
    s_localRuntimeSeq = 0;
    s_loadTimeIdSeq = 0;
    s_networkTickAccum = 0.0f;
    s_validationTickAccum = 0.0f;
    s_digestAccum = 0.0f;
    s_preLiveQueue.clear();

    // Clear per-entity delta state so the new level starts fresh.
    // This also releases the lastSentPayload memory for old entities.
    s_pendingEntityStates.clear();
    s_entityUpdateCache.clear();
    s_relayQueue.clear();
    s_updateQueue.clear();
    s_entityReceivedHash.clear();
    s_lateJoiners.clear();
    s_recentlyDespawned.clear();
    s_unconfirmedEntities.clear();

    // Clear file-scope implementation caches.
    g_identicalSentCount.clear();
    g_pendingSpawns.clear();
}

void NetworkManager::OnLevelLoaded() {
    assert(s_isActive);

    if (s_isServer) {
        NetPacket pkt(PacketType::LevelLoadComplete);
        auto bytes = FinalizeOutbound(pkt);
        const auto compressed = ByteCompressor::CompressData(bytes);
        s_transport->Broadcast(compressed.data(), compressed.size(), true);

        std::fprintf(stdout,
            "[NetworkManager] PT_LevelLoadComplete sent, awaiting %zu client(s)\n",
            s_pendingReadyClients.size());

        if (s_pendingReadyClients.empty())
            OnLevelReady();
    }
    else {
        NetPacket pkt(PacketType::ClientReady);
        SendReliableNow(FinalizeOutbound(pkt), 0);
        std::fprintf(stdout, "[NetworkManager] PT_ClientReady sent\n");
    }
}

void NetworkManager::OnLevelReady() {
    s_isLoadingLevel = false;
    s_networkTickAccum = 0.0f;
    s_digestAccum = 0.0f;

    std::fprintf(stdout,
        "[NetworkManager] Level ready — firing OnNetworkSpawn on %zu entities\n",
        s_entities.size());

    for (auto& [id, entity] : s_entities)
        entity->OnNetworkSpawn();

    FlushPreLiveQueue();
}

void NetworkManager::FlushPreLiveQueue() {
    std::vector<QueuedPacket> queue = std::move(s_preLiveQueue);
    s_preLiveQueue.clear();

    for (auto& qp : queue)
        OnPacketReceived(qp.senderId, qp.buffer.data(), qp.buffer.size());
}

// ---------------------------------------------------------------------------
// Entity registration
// ---------------------------------------------------------------------------

void NetworkManager::Register(NetworkedEntity* entity) {
    assert(entity);
    assert(entity->networkId != 0);
    assert(s_entities.find(entity->networkId) == s_entities.end()
        && "Register: duplicate networkId");
    s_entities[entity->networkId] = entity;
}

void NetworkManager::Unregister(uint64_t networkId) {
    s_entities.erase(networkId);
    s_entityUpdateCache.erase(networkId);    // free lastSentPayload memory
    s_pendingEntityStates.erase(networkId);
    s_entityReceivedHash.erase(networkId);

    // Clear file-scope per-entity caches.
    g_identicalSentCount.erase(networkId);

    // Remove any pending-spawn entry so we don't send a spawn packet for an
    // entity that was destroyed between BroadcastSpawn and the next network tick.
    g_pendingSpawns.erase(
        std::remove_if(g_pendingSpawns.begin(), g_pendingSpawns.end(),
            [networkId](const PendingSpawn& ps) {
                return ps.entityId == networkId;
            }),
        g_pendingSpawns.end());

    // Record a positive "this really was despawned" confirmation, server-side
    // only.  This is the single funnel point for every removal regardless of
    // who initiated it (Level::RemoveEntity, owner disconnect, etc.), so it's
    // the right place to make Case B's destructive correction safe — see the
    // class-level comment "Bidirectional reconciliation safety net".
    if (s_isServer)
        s_recentlyDespawned[networkId] = 0.0f;

    // Any pending "unconfirmed, might need a resend request" tracking for
    // this ID is moot now — it's confirmed gone.
    s_unconfirmedEntities.erase(networkId);
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
    const uint32_t id = NETWORK_ID_RUNTIME_OFFSET + s_localRuntimeSeq++;
    const uint64_t networkId = PackNetworkId(id, ownerId, 0, 0);
    assert(networkId != 0);
    return networkId;
}

uint64_t NetworkManager::MakeLoadPhaseId(const std::string& /*entityId*/) {
    return ++s_loadTimeIdSeq + 1;
}

// ---------------------------------------------------------------------------
// Broadcast helpers
// ---------------------------------------------------------------------------

void NetworkManager::BroadcastSpawn(NetworkedEntity* entity) {
    assert(entity && entity->networkId != 0);

    // FIX: deferred send.  Queue the spawn for the next network tick so that
    // FlushUpdateQueue calls NetSerialize with the fully-settled post-physics
    // state rather than the T=0 data captured at the moment of spawn.
    // This eliminates the visible position snap that occurred when the first
    // EntityUpdate arrived (one frame later) with a slightly different transform.
    //
    // FlushUpdateQueue drains g_pendingSpawns at the top of every flush cycle,
    // before processing regular entity updates, so the spawn and the first
    // EntityUpdate can be bundled in the same compressed batch if they happen
    // to land on the same tick.
    g_pendingSpawns.push_back({ entity->networkId,
        s_isServer ? BROADCAST_PEER_ID : uint8_t(0) });
}

void NetworkManager::BroadcastDespawn(uint64_t networkId) {
    NetPacket pkt(PacketType::DespawnEntity);
    pkt.WriteUInt64(networkId);
    auto bytes = FinalizeOutbound(pkt);
    s_isServer ? SendReliableNow(bytes, BROADCAST_PEER_ID)
        : SendReliableNow(bytes, 0);
}

void NetworkManager::BroadcastOwnerChange(uint64_t networkId, uint8_t newOwner) {
    NetPacket pkt(PacketType::OwnerChange);
    pkt.WriteUInt64(networkId);
    pkt.WriteUInt8(newOwner);
    auto bytes = FinalizeOutbound(pkt);
    s_isServer ? SendReliableNow(bytes, BROADCAST_PEER_ID)
        : SendReliableNow(bytes, 0);
}

// ---------------------------------------------------------------------------
// SendFullSnapshotTo
//
// Sends the full state of every entity to targetPeerId.  Always bypasses
// delta compression — every entity's NetSerialize is called fresh regardless
// of what the cache says.  This guarantees correct state even for entities
// that have been suppressed for the entire session so far.
//
// After a BROADCAST send (validation tick), idle timers are reset for all
// entities: clients just received fresh state, so the next heartbeat
// resend would be redundant immediately after.
// ---------------------------------------------------------------------------

void NetworkManager::SendFullSnapshotTo(uint8_t targetPeerId) {
    assert(s_isServer);

    NetPacket pkt(PacketType::FullSnapshot);
    pkt.WriteUInt32(static_cast<uint32_t>(s_entities.size()));

    for (const auto& [id, entity] : s_entities) {
        pkt.WriteUInt64(entity->networkId);
        pkt.WriteUInt16(NameToIndex(entity->GetClassName()));
        pkt.WriteUInt8(entity->networkOwner);

        // Fresh serialise — not the cached payload.
        // This is intentional: we want the absolute current state even if
        // the entity has been delta-suppressed.
        NetPacket entityData(PacketType::EntityUpdate);
        entity->NetSerialize(entityData);
        const auto& stateBytes = entityData.GetPayloadBytes();

        // GUARD: the per-entity payload size is encoded as uint16 in the
        // FullSnapshot wire format.  If any entity's serialised state exceeds
        // 65 535 bytes, the cast silently truncates the written size while the
        // loop below still writes all bytes.  The receiver then reads fewer bytes
        // than are present, leaving its read cursor inside this entity's data.
        // Every subsequent entity in the snapshot is then parsed from the wrong
        // offset — silent, total snapshot corruption from this entity onward.
        //
        // This should never fire in a well-designed game (entities should carry
        // lightweight transforms and references, not bulk data).  If it does,
        // the fix is to reduce the entity's NetSerialize output, not to widen
        // the field (which would be a wire-format breaking change).
        assert(stateBytes.size() <= 0xFFFF &&
            "Entity serialised state exceeds 65535 bytes — FullSnapshot would "
            "be corrupted from this entity onward.  Reduce NetSerialize output.");

        pkt.WriteUInt16(static_cast<uint16_t>(stateBytes.size()));
        for (uint8_t b : stateBytes)
            pkt.WriteUInt8(b);
    }

    SendReliableNow(FinalizeOutbound(pkt), targetPeerId);

    // After a full broadcast snapshot all clients have fresh state.
    // Reset idle timers so the next heartbeat doesn't immediately duplicate
    // what was just sent.  Skip this for targeted (late-join) sends because
    // other connected clients are unaffected.
    if (targetPeerId == BROADCAST_PEER_ID) {
        for (auto& [id, cache] : s_entityUpdateCache)
            cache.timeSinceLastSent = 0.0f;
    }
}
// ---------------------------------------------------------------------------
// RPC
// ---------------------------------------------------------------------------

void NetworkManager::SendRPC(uint64_t networkId, uint8_t rpcId,
    NetPacket& args, RPCTarget target)
{
    NetPacket pkt(PacketType::RPC);
    pkt.WriteUInt64(networkId);
    pkt.WriteUInt8(rpcId);
    pkt.WriteUInt8(static_cast<uint8_t>(target));
    for (uint8_t b : args.GetPayloadBytes())
        pkt.WriteUInt8(b);

    auto bytes = FinalizeOutbound(pkt);

    if (s_isServer) {
        if (target != RPCTarget::Others) {
            if (NetworkedEntity* entity = Find(networkId)) {
                NetPacket argsCopy = args.RewindedCopy();
                entity->OnRPC(rpcId, argsCopy);
            }
        }
        switch (target) {
        case RPCTarget::All:    RelayReliableAll(bytes);            break;
        case RPCTarget::Others: RelayReliableAll(bytes);            break;
        case RPCTarget::Server:                                     break;
        }
    }
    else {
        SendReliableNow(bytes, 0);
    }
}

// ---------------------------------------------------------------------------
// SpawnForPlayer
// ---------------------------------------------------------------------------

void NetworkManager::SpawnForPlayer(NetworkedEntity* entity, uint8_t targetPeerId) {
    assert(s_isServer);
    assert(entity);

    entity->networkOwner = targetPeerId;
    entity->networkId = AllocateRuntimeId(s_localPeerId);
    entity->isOwned = false;

    Register(entity);
    BroadcastSpawn(entity); // deferred — queued for next tick with fresh state
    entity->OnNetworkSpawn();
}

// ---------------------------------------------------------------------------
// NetworkStat
// ---------------------------------------------------------------------------

NetworkStat NetworkManager::GetStat() {
    return s_transport ? s_transport->GetStat() : NetworkStat{};
}

// ---------------------------------------------------------------------------
// OnPacketReceived — decompresses a bundle and dispatches each inner packet
// ---------------------------------------------------------------------------

void NetworkManager::OnPacketReceived(uint8_t senderId,
    const uint8_t* buffer, size_t length)
{
    std::vector<uint8_t> decompressed;
    try {
        decompressed = ByteCompressor::DecompressData(
            std::vector<uint8_t>(buffer, buffer + length));
    }
    catch (const std::exception& e) {
        std::fprintf(stderr,
            "[NetworkManager] Decompress failed from peer %u: %s\n",
            senderId, e.what());
        return;
    }

    size_t offset = 0;
    while (offset < decompressed.size()) {
        const uint8_t* ptr = decompressed.data() + offset;
        const size_t   remaining = decompressed.size() - offset;

        if (remaining < NetPacket::HEADER_SIZE) {
            std::fprintf(stderr,
                "[NetworkManager] Truncated header from peer %u at offset %zu\n",
                senderId, offset);
            break;
        }

        const uint32_t payloadLen =
            (uint32_t(ptr[4]) << 24) | (uint32_t(ptr[5]) << 16) |
            (uint32_t(ptr[6]) << 8) | uint32_t(ptr[7]);
        const size_t packetSize = NetPacket::HEADER_SIZE + payloadLen;

        if (remaining < packetSize) {
            std::fprintf(stderr,
                "[NetworkManager] Truncated packet from peer %u (need %zu have %zu)\n",
                senderId, packetSize, remaining);
            break;
        }

        NetPacket packet;
        if (!NetPacket::Parse(ptr, packetSize, packet)) {
            std::fprintf(stderr,
                "[NetworkManager] Bad checksum from peer %u at offset %zu\n",
                senderId, offset);
            break;
        }
        offset += packetSize;

        const PacketType type = packet.GetType();

        if (!IsHandshakePacket(type) && s_isLoadingLevel) {
            QueuedPacket qp;
            qp.senderId = senderId;
            qp.buffer = ByteCompressor::CompressData(
                std::vector<uint8_t>(ptr, ptr + packetSize));
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
    switch (packet.GetType()) {

        // ── Handshake ─────────────────────────────────────────────────────

    case PacketType::PeerIdAssign: {
        s_localPeerId = packet.ReadUInt8();
        std::fprintf(stdout, "[NetworkManager] Assigned peerId=%u\n", s_localPeerId);
        break;
    }

    case PacketType::LevelInfo: {
        const std::string levelName = packet.ReadString();
        std::fprintf(stdout, "[NetworkManager] Level info: '%s'\n", levelName.c_str());
        break;
    }

    case PacketType::LevelLoadComplete: {
        NetPacket reply(PacketType::ClientReady);
        SendReliableNow(FinalizeOutbound(reply), 0);
        std::fprintf(stdout, "[NetworkManager] Sent PT_ClientReady\n");
        break;
    }

    case PacketType::ClientReady: {
        assert(s_isServer);

        if (s_lateJoiners.erase(senderId)) {
            // ── Late joiner: level is already live ──────────────────────
            //
            // We do NOT call OnLevelReady() server-side (it would fire
            // OnNetworkSpawn again on all already-live entities).
            // Instead, send the late joiner:
            //   1. PT_LevelReady  — so the client exits its loading state.
            //   2. PT_FullSnapshot — so the client sees all existing entities
            //      including those that have been delta-suppressed.
            //      This also populates the client's s_entityReceivedHash so
            //      its very first digest is already authoritative.

            NetPacket ready(PacketType::LevelReady);
            SendReliableNow(FinalizeOutbound(ready), senderId);

            SendFullSnapshotTo(senderId); // bypasses delta cache, always fresh

            std::fprintf(stdout,
                "[NetworkManager] Late joiner %u ready — targeted snapshot sent\n",
                senderId);
        }
        else {
            // ── Initial load ─────────────────────────────────────────────
            s_pendingReadyClients.erase(senderId);

            std::fprintf(stdout,
                "[NetworkManager] PT_ClientReady from peer %u (%zu remaining)\n",
                senderId, s_pendingReadyClients.size());

            if (s_pendingReadyClients.empty()) {
                NetPacket ready(PacketType::LevelReady);
                auto bytes = FinalizeOutbound(ready);
                const auto compressed = ByteCompressor::CompressData(bytes);
                s_transport->Broadcast(compressed.data(), compressed.size(), true);
                OnLevelReady();
            }
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

                                 // ── Entity lifecycle ──────────────────────────────────────────────

    case PacketType::SpawnEntity: {
        const uint64_t networkId = packet.ReadUInt64();
        const uint16_t wireIndex = packet.ReadUInt16();
        const uint8_t  networkOwner = packet.ReadUInt8();

        if (s_isServer) {
            NetPacket relay(PacketType::SpawnEntity);
            relay.WriteUInt64(networkId);
            relay.WriteUInt16(wireIndex);
            relay.WriteUInt8(networkOwner);
            packet.CopyRemainingTo(relay);
            RelayReliableExcept(senderId, FinalizeOutbound(relay));
        }

        if (Find(networkId)) break;

        const std::string& className = IndexToName(wireIndex);
        if (className.empty()) break;

        Entity* raw = LevelObjectFactory::instance().create(className);
        if (!raw) break;

        auto* entity = dynamic_cast<NetworkedEntity*>(raw);
        if (!entity) { delete raw; break; }

        entity->networkId = networkId;
        entity->networkOwner = networkOwner;
        entity->NetDeserialize(packet);

        if (!s_isServer) {
            // Record the post-deserialize state hash immediately.
            // Without this, the client's next digest would report hash=0 for
            // this entity (not yet in s_entityReceivedHash), the server would
            // see a mismatch against its cache, and send a redundant
            // EntityUpdate correction right after the spawn that just fixed
            // it.  Re-serializing here is cheap (one entity, once) and keeps
            // the digest converged immediately instead of one cycle late.
            NetPacket freshState(PacketType::EntityUpdate);
            freshState.WriteUInt64(networkId);
            entity->NetSerialize(freshState);
            const auto& fresh = freshState.GetPayloadBytes();
            s_entityReceivedHash[networkId] = HashPayload(fresh.data(), fresh.size());
        }

        if (s_level) s_level->AddEntity(entity);

        entity->LoadAssetsIfNeeded();
        entity->Start();

        break;
    }

    case PacketType::DespawnEntity: {
        const uint64_t networkId = packet.ReadUInt64();

        if (s_isServer) {
            NetPacket relay(PacketType::DespawnEntity);
            relay.WriteUInt64(networkId);
            RelayReliableExcept(senderId, FinalizeOutbound(relay));
        }

        if (NetworkedEntity* entity = Find(networkId))
            if (s_level) s_level->RemoveEntity(entity);
        break;
    }

    case PacketType::EntityUpdate: {
        // Hash the FULL payload (networkId + state) BEFORE advancing the cursor.
        // This hash is used for:
        //   • Server: tracking what clients should have for client-owned entities
        //             (stored in s_entityUpdateCache so ProcessEntityDigest can
        //              reference it when the client sends a digest).
        //   • Client: recording the last received hash in s_entityReceivedHash
        //             (included in the next PT_EntityDigest).
        //
        // Both sides compute the same hash from the same bytes, so the digest
        // comparison (client's receivedHash vs server's lastSentHash) works
        // even across the relay path.
        const uint32_t payloadHash = HashPayload(
            packet.GetPayloadBytes().data(),
            packet.GetPayloadBytes().size());

        const uint64_t networkId = packet.ReadUInt64();

        if (s_isServer) {
            // Record hash + payload for client-owned entity relay so
            // ProcessEntityDigest has a reference if a client later reports
            // a mismatched hash for this entity.
            //
            // NOTE: lastSentPayload is also used by the validation-tick
            // reapply loop (Phase 5 of Tick) to snap the server's local
            // representation back to the client's truth at kValidationInterval,
            // fixing the ~1 s snap-back delay that was caused by client-side
            // delta suppression holding off the next resend until
            // kIdleResendInterval.
            auto& cache = s_entityUpdateCache[networkId];
            cache.lastSentHash = payloadHash;
            cache.lastSentPayload = packet.GetPayloadBytes(); // full payload
            cache.everSent = true;
            cache.timeSinceLastSent = 0.0f;

            // Queue for batched relay (FlushUpdateQueue groups by excludePeerId).
            NetPacket relay(PacketType::EntityUpdate);
            relay.WriteUInt64(networkId);
            packet.CopyRemainingTo(relay);
            s_relayQueue.push_back({ FinalizeOutbound(relay), senderId });
        }

        NetworkedEntity* entity = Find(networkId);
        if (entity && !entity->isOwned) {
            entity->NetDeserialize(packet);
            if (!s_isServer) {
                // Record so this hash is included in the next digest.
                s_entityReceivedHash[networkId] = payloadHash;
            }
        }
        break;
    }

                                 // ── Reconciliation digest ─────────────────────────────────────────

    case PacketType::EntityDigest: {
        // Only the server processes digests; clients never send them to each
        // other.  If this arrives on a client it's a no-op (shouldn't happen).
        if (s_isServer)
            ProcessEntityDigest(senderId, packet);
        break;
    }

    case PacketType::SpawnRequest: {
        // Server is telling us it has no record of this entity after waiting
        // well past any plausible delivery delay — most likely our original
        // SpawnEntity was genuinely lost (e.g. a reconnect that broke the
        // reliable channel's delivery guarantee), not just slow.
        //
        // Only act if we still have the entity AND still own it — if we've
        // since destroyed it ourselves, or ownership moved on, there is
        // nothing to resend and the server's own reconciliation will settle
        // once it sees the corresponding despawn/owner-change instead.
        const uint64_t networkId = packet.ReadUInt64();
        if (NetworkedEntity* entity = Find(networkId)) {
            if (entity->isOwned) {
                std::fprintf(stdout,
                    "[NetworkManager] Server requested resend of entity %llu — "
                    "re-broadcasting spawn\n",
                    static_cast<unsigned long long>(networkId));

                // Bypass the deferred spawn queue — the server already waited
                // kSpawnResendRequestDelay before asking, so we send immediately
                // with the current state.
                NetPacket spawnPkt(PacketType::SpawnEntity);
                spawnPkt.WriteUInt64(entity->networkId);
                spawnPkt.WriteUInt16(NameToIndex(entity->GetClassName()));
                spawnPkt.WriteUInt8(entity->networkOwner);
                entity->NetSerialize(spawnPkt);
                auto spawnBytes = FinalizeOutbound(spawnPkt);
                SendReliableNow(spawnBytes,
                    s_isServer ? BROADCAST_PEER_ID : uint8_t(0));
            }
        }
        break;
    }

                                 // ── RPC ───────────────────────────────────────────────────────────

    case PacketType::RPC: {
        const uint64_t  networkId = packet.ReadUInt64();
        const uint8_t   rpcId = packet.ReadUInt8();
        const RPCTarget target = static_cast<RPCTarget>(packet.ReadUInt8());

        if (s_isServer) {
            NetPacket relay(PacketType::RPC);
            relay.WriteUInt64(networkId);
            relay.WriteUInt8(rpcId);
            relay.WriteUInt8(static_cast<uint8_t>(target));
            packet.CopyRemainingTo(relay);
            auto relayBytes = FinalizeOutbound(relay);

            switch (target) {
            case RPCTarget::All:    RelayReliableAll(relayBytes);              break;
            case RPCTarget::Others: RelayReliableExcept(senderId, relayBytes); break;
            case RPCTarget::Server:                                             break;
            }
        }

        if (NetworkedEntity* entity = Find(networkId))
            entity->OnRPC(rpcId, packet);
        break;
    }

    case PacketType::OwnerChange: {
        const uint64_t networkId = packet.ReadUInt64();
        const uint8_t  newOwner = packet.ReadUInt8();

        if (s_isServer) {
            NetPacket relay(PacketType::OwnerChange);
            relay.WriteUInt64(networkId);
            relay.WriteUInt8(newOwner);
            RelayReliableAll(FinalizeOutbound(relay));
        }

        if (NetworkedEntity* entity = Find(networkId))
            if (entity->CanMigrateOwner) {
                entity->networkOwner = newOwner;
                entity->isOwned = (newOwner == s_localPeerId);
            }
        break;
    }

    default:
        std::fprintf(stderr,
            "[NetworkManager] Unknown packet type %u from peer %u\n",
            static_cast<unsigned>(packet.GetType()), senderId);
        break;
    }
}

// ---------------------------------------------------------------------------
// OnEntityListReceived  (processes PT_FullSnapshot on the client)
//
// In addition to the existing spawn/despawn/update logic, records the
// stateHash for every received entity in s_entityReceivedHash so the
// client's very first PT_EntityDigest (sent 500 ms later) is already
// authoritative and won't trigger spurious corrections.
// ---------------------------------------------------------------------------

void NetworkManager::OnEntityListReceived(uint8_t /*senderId*/, NetPacket& packet) {
    assert(!s_isServer);

    const uint32_t entityCount = packet.ReadUInt32();

    std::set<uint64_t>           snapshotIds;
    std::vector<NetworkedEntity*> newlySpawned;

    for (uint32_t i = 0; i < entityCount; ++i) {
        const uint64_t networkId = packet.ReadUInt64();
        const uint16_t wireIndex = packet.ReadUInt16();
        const uint8_t  networkOwner = packet.ReadUInt8();
        const uint16_t payloadSize = packet.ReadUInt16();

        // Read entity state into a temporary buffer.
        NetPacket entityBuffer(PacketType::EntityUpdate);
        for (uint16_t p = 0; p < payloadSize; ++p)
            entityBuffer.WriteUInt8(packet.ReadUInt8());

        // Hash the state bytes (just the entity state, no networkId prefix here)
        // so we store what the client considers the current state.
        // We'll re-use the full EntityUpdate payload convention below.
        const auto& stateBytes = entityBuffer.GetPayloadBytes();

        // Build the canonical payload (networkId + state) to match what the
        // server stores in EntityUpdateCache.lastSentPayload.
        std::vector<uint8_t> canonicalPayload;
        canonicalPayload.reserve(8 + stateBytes.size());
        // networkId big-endian
        for (int b = 7; b >= 0; --b)
            canonicalPayload.push_back(static_cast<uint8_t>((networkId >> (b * 8)) & 0xFF));
        canonicalPayload.insert(canonicalPayload.end(), stateBytes.begin(), stateBytes.end());

        const uint32_t stateHash = HashPayload(
            canonicalPayload.data(), canonicalPayload.size());

        NetPacket readPkt = entityBuffer.RewindedCopy();

        snapshotIds.insert(networkId);

        NetworkedEntity* existing = Find(networkId);
        if (existing) {
            existing->networkOwner = networkOwner;
            existing->isOwned = (networkOwner == s_localPeerId);
            if (!existing->isOwned)
                existing->NetDeserialize(readPkt);
        }
        else {
            const std::string& className = IndexToName(wireIndex);
            if (className.empty()) continue;

            Entity* raw = LevelObjectFactory::instance().create(className);
            if (!raw) continue;

            auto* entity = dynamic_cast<NetworkedEntity*>(raw);
            if (!entity) { delete raw; continue; }

            entity->networkId = networkId;
            entity->networkOwner = networkOwner;
            entity->isOwned = (networkOwner == s_localPeerId);

            entity->NetDeserialize(readPkt);


            if (s_level) s_level->AddEntity(entity);

            if (!s_isLoadingLevel) {
                entity->LoadAssetsIfNeeded();
                entity->Start();
            }

            if (!s_isLoadingLevel) newlySpawned.push_back(entity);
        }

        // Record the received hash.  The client will include this in its next
        // digest so the server won't send spurious corrections for entities
        // that are fully up-to-date after the snapshot.
        s_entityReceivedHash[networkId] = stateHash;
    }

    // Remove local entities absent from the authoritative list.
    std::vector<NetworkedEntity*> toRemove;
    for (const auto& [id, entity] : s_entities) {
        if (snapshotIds.count(id) || entity->isOwned) continue;
        toRemove.push_back(entity);
    }
    for (NetworkedEntity* entity : toRemove) {
        std::fprintf(stdout,
            "[NetworkManager] Snapshot: removing stale entity %u\n",
            entity->networkId);
        if (s_level) {
            s_isLoadingLevel ? s_level->RemoveEntitySilent(entity)
                : s_level->RemoveEntity(entity);
        }
    }

    for (NetworkedEntity* entity : newlySpawned)
        entity->OnNetworkSpawn();
}

// ---------------------------------------------------------------------------
// OnPeerConnected / OnPeerDisconnected
// ---------------------------------------------------------------------------

void NetworkManager::OnPeerConnected(uint8_t peerId) {
    std::fprintf(stdout, "[NetworkManager] Peer connected: %u\n", peerId);
    if (!s_isServer) return;

    // Assign the peer its ID.
    {
        NetPacket pkt(PacketType::PeerIdAssign);
        pkt.WriteUInt8(peerId);
        SendReliableNow(FinalizeOutbound(pkt), peerId);
    }

    if (s_isLoadingLevel) {
        // Connected during the load sequence — normal initial-load path.
        s_pendingReadyClients.insert(peerId);
    }
    else {
        // Connected while the session is already live.
        // Track in s_lateJoiners (separate from s_pendingReadyClients) so the
        // "are all initial-load clients ready?" check is not affected.
        s_lateJoiners.insert(peerId);

        // Tell the client what level to load.
        NetPacket pkt(PacketType::LevelInfo);
        pkt.WriteString(s_level ? s_level->filePath : "");
        SendReliableNow(FinalizeOutbound(pkt), peerId);

        std::fprintf(stdout,
            "[NetworkManager] Late joiner peer %u — sent LevelInfo, awaiting ClientReady\n",
            peerId);
    }
}

void NetworkManager::OnPeerDisconnected(uint8_t peerId) {
    std::fprintf(stdout, "[NetworkManager] Peer disconnected: %u\n", peerId);

    s_pendingReadyClients.erase(peerId);
    s_lateJoiners.erase(peerId);

    if (!s_isServer || !s_level) return;

    std::vector<uint64_t> toDestroy;
    std::vector<uint64_t> toMigrate;

    for (const auto& [id, entity] : s_entities) {
        if (entity->networkOwner != peerId) continue;
        (!entity->DestroyOnOwnerDisconnect && entity->CanMigrateOwner)
            ? toMigrate.push_back(id)
            : toDestroy.push_back(id);
    }

    for (uint64_t id : toDestroy) {
        BroadcastDespawn(id);
        if (NetworkedEntity* e = Find(id))
            s_level->RemoveEntitySilent(e);
        Unregister(id);
    }

    for (uint64_t id : toMigrate) {
        if (NetworkedEntity* entity = Find(id)) {
            entity->networkOwner = 0;
            entity->isOwned = (s_localPeerId == 0);
            BroadcastOwnerChange(id, 0);
            std::fprintf(stdout,
                "[NetworkManager] Entity %u migrated to server after peer %u left\n",
                id, peerId);
        }
    }
}

// ---------------------------------------------------------------------------
// DrawDebugUi
// ---------------------------------------------------------------------------

void NetworkManager::DrawDebugUi() {
    const float PAD = 10.0f;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(
        ImVec2(viewport->WorkPos.x + viewport->WorkSize.x - PAD,
            viewport->WorkPos.y + PAD),
        ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.5f);

    constexpr ImGuiWindowFlags kWinFlags =
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoMove;

    if (!ImGui::Begin("Network Stats", nullptr, kWinFlags)) {
        ImGui::End();
        return;
    }

    auto netStat = GetStat();

    // ── Connection health ──────────────────────────────────────────────
    ImGui::TextDisabled("Connection Health");
    ImGui::Separator();

    auto pingColor = [](uint32_t rtt) {
        if (rtt > 150) return ImVec4(1.f, 0.2f, 0.2f, 1.f);
        if (rtt > 80) return ImVec4(1.f, 1.f, 0.2f, 1.f);
        return ImVec4(0.2f, 1.f, 0.2f, 1.f);
        };

    ImGui::Text("Ping:"); ImGui::SameLine(90);
    ImGui::TextColored(pingColor(netStat.roundTripTime),
        "%u ms (+/- %u ms)", netStat.roundTripTime, netStat.roundTripTimeVariance);

    ImGui::Text("Loss:"); ImGui::SameLine(90);
    {
        ImVec4 lc = netStat.packetLossPercent > 5.f
            ? ImVec4(1.f, 0.2f, 0.2f, 1.f)
            : (netStat.packetLossPercent > 1.f
                ? ImVec4(1.f, 1.f, 0.2f, 1.f)
                : ImVec4(0.2f, 1.f, 0.2f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, lc);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.2f%%", netStat.packetLossPercent);
        ImGui::ProgressBar(
            std::min(netStat.packetLossPercent / 20.f, 1.f),
            ImVec2(180.f, 0.f), buf);
        ImGui::PopStyleColor();
    }
    ImGui::Spacing();

    // ── Data usage ────────────────────────────────────────────────────
    ImGui::TextDisabled("Data Usage");
    ImGui::Separator();

    auto fmtBytes = [](uint64_t b) -> std::string {
        char buf[64];
        if (b < 1024)         std::snprintf(buf, sizeof(buf), "%llu B", b);
        else if (b < 1 << 20)   std::snprintf(buf, sizeof(buf), "%.2f KB", b / 1024.0);
        else                  std::snprintf(buf, sizeof(buf), "%.2f MB", b / 1048576.0);
        return buf;
        };
    ImGui::Text("RX: %s", fmtBytes(netStat.incomingBytesTotal).c_str());
    ImGui::Text("TX: %s", fmtBytes(netStat.outgoingBytesTotal).c_str());
    ImGui::Spacing();

    // ── Delta compression ─────────────────────────────────────────────
    ImGui::TextDisabled("Delta Compression (last tick)");
    ImGui::Separator();

    const uint32_t total = s_deltaStats.entitiesTotal;
    const uint32_t skipped = s_deltaStats.entitiesSkipped;
    const float    ratio = total > 0 ? float(skipped) / float(total) * 100.f : 0.f;

    ImGui::Text("%u/%u entities sent  (%.0f%% suppressed)",
        s_deltaStats.entitiesSent, total, ratio);
    {
        ImVec4 dc(std::max(0.f, 1.f - ratio / 50.f),
            std::min(1.f, ratio / 50.f), 0.2f, 1.f);
        ImGui::PushStyleColor(ImGuiCol_PlotHistogram, dc);
        char buf[16]; std::snprintf(buf, sizeof(buf), "%.1f%%", ratio);
        ImGui::ProgressBar(ratio / 100.f, ImVec2(180.f, 0.f), buf);
        ImGui::PopStyleColor();
    }
    ImGui::Spacing();

    // ── Reconciliation digest ─────────────────────────────────────────
    ImGui::TextDisabled("Reconciliation (last digest cycle)");
    ImGui::Separator();

    if (s_isServer) {
        ImGui::Text("Missing fixed:  %u", s_deltaStats.digestMissing);
        ImGui::Text("Stale fixed:    %u", s_deltaStats.digestStale);
        ImGui::Text("Phantom fixed:  %u (confirmed despawns only)",
            s_deltaStats.digestPhantom);

        // Unconfirmed entities: IDs a client reported that the server
        // doesn't have, but hasn't confirmed despawned either — these are
        // tracked, never destroyed, and only escalated to a SpawnRequest
        // after kSpawnResendRequestDelay. Non-zero here briefly during a
        // spawn is normal; non-zero for a long time means a spawn may have
        // genuinely been lost.
        if (!s_unconfirmedEntities.empty()) {
            ImVec4 warnColor(1.f, 0.7f, 0.2f, 1.f);
            ImGui::TextColored(warnColor, "Unconfirmed: %zu (awaiting spawn)",
                s_unconfirmedEntities.size());
        }
        else {
            ImGui::TextDisabled("Unconfirmed: 0");
        }
    }
    else {
        ImGui::Text("Digest interval: %.0f ms  (%.0f Hz)",
            kDigestInterval * 1000.f, 1.f / kDigestInterval);
        ImGui::Text("Entities tracked: %zu", s_entityReceivedHash.size());
    }

    ImGui::End();
}