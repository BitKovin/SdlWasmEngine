#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include <glm.h>

// ---------------------------------------------------------------------------
// PacketType / RPCTarget enums (§5)
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
};

enum class RPCTarget : uint8_t {
    Server  = 0,
    All     = 1,
    Others  = 2,
};

// ---------------------------------------------------------------------------
// NetPacket (§5)
//
// Single serialization and transport container.
// Header layout (10 bytes, always present):
//   uint8_t   packetType
//   uint8_t   senderId
//   uint16_t  sequenceNumber
//   uint16_t  payloadLength
//   uint16_t  checksum         (CRC-16 over type+senderId+seq+payloadLen+payload)
// ---------------------------------------------------------------------------

class NetPacket {
public:
    // ── Construction ──────────────────────────────────────────────────────

    // Outbound packet.
    explicit NetPacket(PacketType type);

    // Default-constructible so it can be used as an out-param in Parse().
    NetPacket() = default;

    // Parse an inbound raw byte buffer.
    // Returns false (and leaves 'out' invalid) on checksum mismatch or
    // truncated header.
    static bool Parse(const uint8_t* buffer, size_t length, NetPacket& out);


    // ── Write (append to payload) ─────────────────────────────────────────

    void WriteBool   (bool           value);
    void WriteInt8   (int8_t         value);
    void WriteUInt8  (uint8_t        value);
    void WriteInt16  (int16_t        value);
    void WriteUInt16 (uint16_t       value);
    void WriteInt32  (int32_t        value);
    void WriteUInt32 (uint32_t       value);
    void WriteFloat  (float          value);
    void WriteVector3(const glm::vec3& value);  // x, y, z as three floats
    void WriteQuat   (const glm::quat& value);  // x, y, z, w as four floats
    void WriteString (const std::string& value); // WriteUInt16(len) + bytes


    // ── Read (consume from payload in order) ──────────────────────────────
    // Asserts in debug builds if reading past end of payload.

    bool        ReadBool();
    int8_t      ReadInt8();
    uint8_t     ReadUInt8();
    int16_t     ReadInt16();
    uint16_t    ReadUInt16();
    int32_t     ReadInt32();
    uint32_t    ReadUInt32();
    float       ReadFloat();
    glm::vec3   ReadVector3();
    glm::quat   ReadQuat();
    std::string ReadString();


    // ── Metadata ──────────────────────────────────────────────────────────

    PacketType GetType()           const;
    uint8_t    GetSenderId()       const;
    uint16_t   GetSequenceNumber() const;
    bool       IsValid()           const;

    void SetSenderId      (uint8_t  id);
    void SetSequenceNumber(uint16_t seq);


    // ── Serialization ─────────────────────────────────────────────────────

    // Finalize: writes payloadLength + checksum into header, returns full bytes.
    // Must be called exactly once after all Write calls.
    std::vector<uint8_t> Finalize();

    // Returns a copy with the read cursor reset to payload start.
    // Used by the server to relay received packets without re-serializing.
    NetPacket RewindedCopy() const;


    // ── Helpers used by NetworkManager ───────────────────────────────────

    // Returns a const reference to the raw payload bytes.
    // Used when copying RPC args into a relay packet.
    const std::vector<uint8_t>& GetPayloadBytes() const;

    // Copies all bytes from the current read cursor to the end of this
    // packet's payload into 'dest' via WriteUInt8 calls.
    // Used when the server needs to relay a packet that it partially read.
    void CopyRemainingTo(NetPacket& dest) const;


    // ── Internal size (header only, no payload) ───────────────────────────
    static constexpr size_t HEADER_SIZE = 10;

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
                                     uint16_t      payloadLen,
                                     const uint8_t* payload,
                                     size_t         payloadSize);

    // Low-level write helpers (little-endian)
    void AppendU8 (uint8_t  v);
    void AppendU16(uint16_t v);
    void AppendU32(uint32_t v);
    void AppendF32(float    v);

    uint8_t  ConsumeU8();
    uint16_t ConsumeU16();
    uint32_t ConsumeU32();
    float    ConsumeF32();

    void AssertCanRead(size_t bytes) const;
};
