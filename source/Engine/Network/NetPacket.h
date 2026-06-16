#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <glm.h>

// ---------------------------------------------------------------------------
// PacketType / RPCTarget enums
// ---------------------------------------------------------------------------

enum class PacketType : uint8_t {
    PeerIdAssign      = 0,
    LevelLoadComplete = 1,
    ClientReady       = 2,
    LevelReady        = 3,
    FullSnapshot      = 4,
    SpawnEntity       = 5,
    DespawnEntity     = 6,
    EntityUpdate      = 7,
    RPC               = 8,
    LevelInfo         = 9,
    OwnerChange       = 10,

    // Client → Server, 2 Hz.
    // Payload: uint16 entityCount + N × (uint64 networkId + uint32 stateHash).
    // Server diffs this against its authoritative state and sends targeted
    // corrections (SpawnEntity / EntityUpdate / DespawnEntity) back to only
    // this client, without touching the normal broadcast update queue.
    EntityDigest      = 11,

    // Server → Client (the entity's owner), reliable.
    // Payload: uint64 networkId.
    // Sent when a client's digest has reported an entity ID the server has
    // no record of for longer than a grace window — i.e. the original
    // SpawnEntity likely never arrived (not just "in flight"). Asks the
    // owning client to resend it. The client looks the ID up locally; if it
    // still owns that entity, it re-broadcasts SpawnEntity for it.
    SpawnRequest      = 12
};

enum class RPCTarget : uint8_t {
    Server  = 0,
    All     = 1,
    Others  = 2,
};

// ---------------------------------------------------------------------------
// NetPacket
//
// Single serialisation and transport container.
//
// Header layout (10 bytes):
//   uint8_t   packetType
//   uint8_t   senderId
//   uint16_t  sequenceNumber   (big-endian)
//   uint32_t  payloadLength    (big-endian)
//   uint16_t  checksum         (CRC-16/CCITT-FALSE over all header fields + payload)
//
// ---------------------------------------------------------------------------

class NetPacket {
public:
    // ── Construction ─────────────────────────────────────────────────────

    explicit NetPacket(PacketType type);
    NetPacket() = default;

    // Parse an inbound raw buffer.  Returns false on checksum mismatch or
    // truncated header; leaves 'out' invalid in that case.
    static bool Parse(const uint8_t* buffer, size_t length, NetPacket& out);


    // ── Write (append to payload) ────────────────────────────────────────

    void WriteBool   (bool             value);
    void WriteInt8   (int8_t           value);
    void WriteUInt8  (uint8_t          value);
    void WriteInt16  (int16_t          value);
    void WriteUInt16 (uint16_t         value);
    void WriteInt32  (int32_t          value);
    void WriteUInt32 (uint32_t         value);
    void WriteUInt64 (uint64_t         value);
    void WriteFloat  (float            value);
    void WriteVector3(const glm::vec3& value);   // 12 bytes: x,y,z as full f32
    void WriteQuat   (const glm::quat& value);   // 16 bytes: x,y,z,w as full f32
    void WriteString (const std::string& value); // uint16 len + bytes


    // ── Quantised write helpers ──────────────────────────────────────────
    //
    // These opt-in helpers trade a small amount of precision for a large
    // reduction in per-entity bandwidth.  Use them in NetSerialize /
    // NetDeserialize pairs; they are completely independent of the full-
    // precision helpers above.
    //
    // WritePackedVec3 — 6 bytes (vs 12 for WriteVector3)
    //   Each axis is quantised to a signed 16-bit integer.
    //   invScale = 1.0 / desired_precision_in_world_units.
    //   Example: invScale = 100.0   → 0.01 m precision, ±327 m range
    //            invScale =  10.0   → 0.10 m precision, ±3276 m range
    //            invScale = 1000.0  → 0.001 m precision, ±32 m range
    //   Matching read: ReadPackedVec3(1.0f / invScale)
    //
    // WritePackedQuat — 4 bytes (vs 16 for WriteQuat)
    //   Smallest-three encoding: drops the largest-magnitude component (always
    //   reconstructable from the unit-quaternion constraint) and quantises the
    //   remaining three to 10 bits each.  Error is ~0.001 rad (~0.06°).
    //   Matching read: ReadPackedQuat()

    void      WritePackedVec3(const glm::vec3& v, float invScale);
    glm::vec3 ReadPackedVec3 (float scale);

    void      WritePackedQuat(const glm::quat& q);
    glm::quat ReadPackedQuat ();


    // ── Read (consume from payload in order) ────────────────────────────
    // Debug-asserts in debug builds if reading past end.

    bool        ReadBool();
    int8_t      ReadInt8();
    uint8_t     ReadUInt8();
    int16_t     ReadInt16();
    uint16_t    ReadUInt16();
    int32_t     ReadInt32();
    uint32_t    ReadUInt32();
    uint64_t    ReadUInt64();
    float       ReadFloat();
    glm::vec3   ReadVector3();
    glm::quat   ReadQuat();
    std::string ReadString();


    // ── Metadata ─────────────────────────────────────────────────────────

    PacketType GetType()           const;
    uint8_t    GetSenderId()       const;
    uint16_t   GetSequenceNumber() const;
    bool       IsValid()           const;

    void SetSenderId      (uint8_t  id);
    void SetSequenceNumber(uint16_t seq);


    // ── Serialisation ────────────────────────────────────────────────────

    // Finalise: stamps payloadLength + checksum, returns the complete wire
    // bytes (header + payload).  Call exactly once, after all Write calls.
    std::vector<uint8_t> Finalize();

    // Build a fully-formed wire packet from a pre-built payload vector
    // without constructing a NetPacket object.  Used by NetworkManager's
    // flush path to avoid allocating/filling a throw-away packet object
    // for each entity update.
    static std::vector<uint8_t> BuildFromPayload(PacketType       type,
                                                  uint8_t          senderId,
                                                  uint16_t         seq,
                                                  const std::vector<uint8_t>& payload);

    // Returns a copy with the read cursor reset to payload start.
    NetPacket RewindedCopy() const;


    // ── Helpers used by NetworkManager ───────────────────────────────────

    const std::vector<uint8_t>& GetPayloadBytes() const;

    void CopyRemainingTo(NetPacket& dest) const;


    // ── Constants ────────────────────────────────────────────────────────
    static constexpr size_t HEADER_SIZE = 10; // 1+1+2+4+2

private:
    PacketType            type_           = PacketType::PeerIdAssign;
    uint8_t               senderId_       = 0;
    uint16_t              sequenceNumber_ = 0;
    bool                  valid_          = true;

    std::vector<uint8_t>  payload_;
    size_t                readCursor_     = 0;

    // CRC-16/CCITT-FALSE (polynomial 0x1021, init 0xFFFF, no reflection)
    static uint16_t ComputeChecksum(PacketType    type,
                                     uint8_t       senderId,
                                     uint16_t      seq,
                                     uint32_t      payloadLen,
                                     const uint8_t* payload,
                                     size_t         payloadSize);

    // Low-level write helpers (big-endian / network byte order)
    void AppendU8 (uint8_t  v);
    void AppendU16(uint16_t v);
    void AppendU32(uint32_t v);
    void AppendU64(uint64_t v);
    void AppendF32(float    v);

    uint8_t  ConsumeU8();
    uint16_t ConsumeU16();
    uint32_t ConsumeU32();
    uint64_t ConsumeU64();
    float    ConsumeF32();

    void AssertCanRead(size_t bytes) const;
};
