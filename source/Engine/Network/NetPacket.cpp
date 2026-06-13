#include "NetPacket.h"

#include <cassert>
#include <cstring>
#include <stdexcept>

// ---------------------------------------------------------------------------
// CRC-16/CCITT-FALSE
// Polynomial: 0x1021 | Init: 0xFFFF | RefIn: false | RefOut: false | XorOut: 0x0000
// ---------------------------------------------------------------------------

uint16_t NetPacket::ComputeChecksum(PacketType    type,
    uint8_t       senderId,
    uint16_t      seq,
    uint32_t      payloadLen, // <-- Changed to uint32_t
    const uint8_t* payload,
    size_t         payloadSize)
{
    auto crc16_update = [](uint16_t crc, uint8_t byte) -> uint16_t {
        crc ^= static_cast<uint16_t>(byte) << 8;
        for (int i = 0; i < 8; ++i) {
            if (crc & 0x8000)
                crc = static_cast<uint16_t>((crc << 1) ^ 0x1021);
            else
                crc = static_cast<uint16_t>(crc << 1);
        }
        return crc;
        };

    uint16_t crc = 0xFFFF;
    crc = crc16_update(crc, static_cast<uint8_t>(type));
    crc = crc16_update(crc, senderId);
    crc = crc16_update(crc, static_cast<uint8_t>(seq >> 8));
    crc = crc16_update(crc, static_cast<uint8_t>(seq & 0xFF));
    // 32-bit shift for payload length
    crc = crc16_update(crc, static_cast<uint8_t>((payloadLen >> 24) & 0xFF));
    crc = crc16_update(crc, static_cast<uint8_t>((payloadLen >> 16) & 0xFF));
    crc = crc16_update(crc, static_cast<uint8_t>((payloadLen >> 8) & 0xFF));
    crc = crc16_update(crc, static_cast<uint8_t>(payloadLen & 0xFF));
    for (size_t i = 0; i < payloadSize; ++i) {
        crc = crc16_update(crc, payload[i]);
    }
    return crc;
}

// ---------------------------------------------------------------------------
// Construction
// ---------------------------------------------------------------------------

NetPacket::NetPacket(PacketType type)
    : type_(type), valid_(true) {}

// ---------------------------------------------------------------------------
// Parse
// ---------------------------------------------------------------------------

bool NetPacket::Parse(const uint8_t* buffer, size_t length, NetPacket& out) {
    if (length < HEADER_SIZE) {
        out.valid_ = false;
        return false;
    }

    out.type_ = static_cast<PacketType>(buffer[0]);
    out.senderId_ = buffer[1];
    out.sequenceNumber_ = static_cast<uint16_t>((buffer[2] << 8) | buffer[3]);

    // Read 32-bit payload length from bytes 4-7
    uint32_t payloadLen = (static_cast<uint32_t>(buffer[4]) << 24) |
        (static_cast<uint32_t>(buffer[5]) << 16) |
        (static_cast<uint32_t>(buffer[6]) << 8) |
        static_cast<uint32_t>(buffer[7]);

    // Checksum moved to bytes 8-9
    uint16_t checksum = static_cast<uint16_t>((buffer[8] << 8) | buffer[9]);

    if (length < HEADER_SIZE + payloadLen) {
        out.valid_ = false;
        return false;
    }

    const uint8_t* payloadPtr = buffer + HEADER_SIZE;

    uint16_t expected = ComputeChecksum(out.type_, out.senderId_,
        out.sequenceNumber_, payloadLen,
        payloadPtr, payloadLen);
    if (expected != checksum) {
        out.valid_ = false;
        return false;
    }

    out.payload_.assign(payloadPtr, payloadPtr + payloadLen);
    out.readCursor_ = 0;
    out.valid_ = true;
    return true;
}

// ---------------------------------------------------------------------------
// Finalize
// ---------------------------------------------------------------------------

std::vector<uint8_t> NetPacket::Finalize() {
    uint32_t payloadLen = static_cast<uint32_t>(payload_.size()); // <-- Now handles > 65KB
    uint16_t checksum = ComputeChecksum(type_, senderId_, sequenceNumber_,
        payloadLen,
        payload_.data(), payload_.size());

    std::vector<uint8_t> out;
    out.reserve(HEADER_SIZE + payload_.size());

    out.push_back(static_cast<uint8_t>(type_));
    out.push_back(senderId_);
    out.push_back(static_cast<uint8_t>(sequenceNumber_ >> 8));
    out.push_back(static_cast<uint8_t>(sequenceNumber_ & 0xFF));
    // Write 32-bit payload length
    out.push_back(static_cast<uint8_t>((payloadLen >> 24) & 0xFF));
    out.push_back(static_cast<uint8_t>((payloadLen >> 16) & 0xFF));
    out.push_back(static_cast<uint8_t>((payloadLen >> 8) & 0xFF));
    out.push_back(static_cast<uint8_t>(payloadLen & 0xFF));
    // Checksum
    out.push_back(static_cast<uint8_t>(checksum >> 8));
    out.push_back(static_cast<uint8_t>(checksum & 0xFF));

    out.insert(out.end(), payload_.begin(), payload_.end());
    return out;
}

// ---------------------------------------------------------------------------
// RewindedCopy
// ---------------------------------------------------------------------------

NetPacket NetPacket::RewindedCopy() const {
    NetPacket copy;
    copy.type_           = type_;
    copy.senderId_       = senderId_;
    copy.sequenceNumber_ = sequenceNumber_;
    copy.valid_          = valid_;
    copy.payload_        = payload_;
    copy.readCursor_     = 0; // rewound
    return copy;
}

// ---------------------------------------------------------------------------
// Metadata
// ---------------------------------------------------------------------------

PacketType NetPacket::GetType()            const { return type_; }
uint8_t    NetPacket::GetSenderId()        const { return senderId_; }
uint16_t   NetPacket::GetSequenceNumber()  const { return sequenceNumber_; }
bool       NetPacket::IsValid()            const { return valid_; }

void NetPacket::SetSenderId      (uint8_t  id)  { senderId_       = id; }
void NetPacket::SetSequenceNumber(uint16_t seq) { sequenceNumber_ = seq; }

// ---------------------------------------------------------------------------
// Helpers used by NetworkManager
// ---------------------------------------------------------------------------

const std::vector<uint8_t>& NetPacket::GetPayloadBytes() const {
    return payload_;
}

void NetPacket::CopyRemainingTo(NetPacket& dest) const {
    for (size_t i = readCursor_; i < payload_.size(); ++i) {
        dest.WriteUInt8(payload_[i]);
    }
}

// ---------------------------------------------------------------------------
// Low-level append helpers (little-endian / network byte order: big-endian)
// We use big-endian (network byte order) for multi-byte values.
// ---------------------------------------------------------------------------

void NetPacket::AppendU8(uint8_t v) {
    payload_.push_back(v);
}

void NetPacket::AppendU16(uint16_t v) {
    payload_.push_back(static_cast<uint8_t>(v >> 8));
    payload_.push_back(static_cast<uint8_t>(v & 0xFF));
}

void NetPacket::AppendU32(uint32_t v) {
    payload_.push_back(static_cast<uint8_t>((v >> 24) & 0xFF));
    payload_.push_back(static_cast<uint8_t>((v >> 16) & 0xFF));
    payload_.push_back(static_cast<uint8_t>((v >>  8) & 0xFF));
    payload_.push_back(static_cast<uint8_t>( v        & 0xFF));
}

void NetPacket::AppendU64(uint64_t v) {
    AppendU32(static_cast<uint32_t>((v >> 32) & 0xFFFFFFFFu));
    AppendU32(static_cast<uint32_t>( v        & 0xFFFFFFFFu));
}

void NetPacket::AppendF32(float v) {
    uint32_t bits;
    std::memcpy(&bits, &v, sizeof(bits));
    AppendU32(bits);
}

// ---------------------------------------------------------------------------
// Low-level consume helpers
// ---------------------------------------------------------------------------

void NetPacket::AssertCanRead(size_t bytes) const {
    assert(readCursor_ + bytes <= payload_.size() &&
           "NetPacket: reading past end of payload");
}

uint8_t NetPacket::ConsumeU8() {
    AssertCanRead(1);
    return payload_[readCursor_++];
}

uint16_t NetPacket::ConsumeU16() {
    AssertCanRead(2);
    uint16_t v = static_cast<uint16_t>((payload_[readCursor_] << 8) |
                                        payload_[readCursor_ + 1]);
    readCursor_ += 2;
    return v;
}

uint32_t NetPacket::ConsumeU32() {
    AssertCanRead(4);
    uint32_t v = (static_cast<uint32_t>(payload_[readCursor_    ]) << 24) |
                 (static_cast<uint32_t>(payload_[readCursor_ + 1]) << 16) |
                 (static_cast<uint32_t>(payload_[readCursor_ + 2]) <<  8) |
                 (static_cast<uint32_t>(payload_[readCursor_ + 3])      );
    readCursor_ += 4;
    return v;
}

uint64_t NetPacket::ConsumeU64() {
    uint64_t hi = ConsumeU32();
    uint64_t lo = ConsumeU32();
    return (hi << 32) | lo;
}

float NetPacket::ConsumeF32() {
    uint32_t bits = ConsumeU32();
    float v;
    std::memcpy(&v, &bits, sizeof(v));
    return v;
}

// ---------------------------------------------------------------------------
// Public write methods
// ---------------------------------------------------------------------------

void NetPacket::WriteBool   (bool    v) { AppendU8(v ? 1 : 0); }
void NetPacket::WriteInt8   (int8_t  v) { AppendU8(static_cast<uint8_t>(v)); }
void NetPacket::WriteUInt8  (uint8_t v) { AppendU8(v); }

void NetPacket::WriteInt16  (int16_t  v) { AppendU16(static_cast<uint16_t>(v)); }
void NetPacket::WriteUInt16 (uint16_t v) { AppendU16(v); }

void NetPacket::WriteInt32  (int32_t  v) { AppendU32(static_cast<uint32_t>(v)); }
void NetPacket::WriteUInt32 (uint32_t v) { AppendU32(v); }
void NetPacket::WriteUInt64 (uint64_t v) { AppendU64(v); }

void NetPacket::WriteFloat  (float    v) { AppendF32(v); }

void NetPacket::WriteVector3(const glm::vec3& v) {
    AppendF32(v.x);
    AppendF32(v.y);
    AppendF32(v.z);
}

void NetPacket::WriteQuat(const glm::quat& v) {
    AppendF32(v.x);
    AppendF32(v.y);
    AppendF32(v.z);
    AppendF32(v.w);
}

void NetPacket::WriteString(const std::string& v) {
    assert(v.size() <= 0xFFFF && "WriteString: string too long");
    AppendU16(static_cast<uint16_t>(v.size()));
    for (char c : v) AppendU8(static_cast<uint8_t>(c));
}

// ---------------------------------------------------------------------------
// Public read methods
// ---------------------------------------------------------------------------

bool        NetPacket::ReadBool()    { return ConsumeU8() != 0; }
int8_t      NetPacket::ReadInt8()    { return static_cast<int8_t>(ConsumeU8()); }
uint8_t     NetPacket::ReadUInt8()   { return ConsumeU8(); }
int16_t     NetPacket::ReadInt16()   { return static_cast<int16_t>(ConsumeU16()); }
uint16_t    NetPacket::ReadUInt16()  { return ConsumeU16(); }
int32_t     NetPacket::ReadInt32()   { return static_cast<int32_t>(ConsumeU32()); }
uint32_t    NetPacket::ReadUInt32()  { return ConsumeU32(); }
uint64_t    NetPacket::ReadUInt64()  { return ConsumeU64(); }
float       NetPacket::ReadFloat()   { return ConsumeF32(); }

glm::vec3 NetPacket::ReadVector3() {
    float x = ConsumeF32();
    float y = ConsumeF32();
    float z = ConsumeF32();
    return glm::vec3(x, y, z);
}

glm::quat NetPacket::ReadQuat() {
    float x = ConsumeF32();
    float y = ConsumeF32();
    float z = ConsumeF32();
    float w = ConsumeF32();
    return glm::quat(w, x, y, z); // glm::quat ctor is (w, x, y, z)
}

std::string NetPacket::ReadString() {
    uint16_t len = ConsumeU16();
    AssertCanRead(len);
    std::string s(reinterpret_cast<const char*>(payload_.data() + readCursor_), len);
    readCursor_ += len;
    return s;
}
