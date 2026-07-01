#pragma once
// ---------------------------------------------------------------------------
// RelayProtocol.h
//
// Internal wire-format definitions shared by RelayTransport_Native.cpp and
// RelayTransport_Emscripten.cpp. NOT part of the public API — never include
// this from RelayTransport.h or any engine/game code.
//
// All multi-byte integers are little-endian on the wire, packed/unpacked
// byte-by-byte below so the format never depends on host endianness. The C#
// relay (RelayServer/Protocol/Wire.cs) uses the exact same byte order.
//
// Framing (how a logical message is delimited on each carrier):
//   TCP : [u32 length][u8 msgType][payload]   (length = 1 + payload.size())
//   UDP : C->S  [u64 token][u8 msgType][payload]
//         S->C  [u8 msgType][payload]              (no token — see below)
//   WS  : [u8 msgType][payload]                     (one binary frame per message)
//
// The UDP token only travels C->S. The relay's UDP socket is shared by every
// connected client, so an inbound datagram needs the token to say which
// session it belongs to. Going the other way, the client's UDP socket only
// ever talks to the relay, so there is nothing to disambiguate — the token
// would be dead weight.
//
// Data message payloads differ by direction for the same reason Send() and
// onPacketReceived() differ: the sender knows *where* to route, the
// recipient only needs *who it came from*.
//   C->S Data: [u8 destMode][u8 destOrExcludePeerId][bytes...]
//   S->C Data: [u8 senderId][bytes...]
// ---------------------------------------------------------------------------

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

namespace relay_protocol {

// UUID string length, e.g. "3fa85f64-5717-4562-b3fc-2c963f66afa6"
constexpr size_t ROOM_CODE_LENGTH = 36;

// Sentinel for "no peer id assigned yet" — 0..254 are valid peer ids.
constexpr uint8_t INVALID_PEER_ID = 0xFF;

enum class MessageType : uint8_t {
    CreateRoom  = 1,  // C->S  [u8 maxClients][u8 closeRoomOnHostLeave]
    JoinRoom    = 2,  // C->S  [36-byte room code]
    LeaveRoom   = 3,  // C->S  (no payload)
    RoomCreated = 4,  // S->C  [36-byte room code][u8 peerId(=0)]
    RoomJoined  = 5,  // S->C  [u8 assignedPeerId][u8 hostPeerId][u8 peerCount][peerCount * u8]
    PeerJoined  = 6,  // S->C  [u8 peerId]
    PeerLeft    = 7,  // S->C  [u8 peerId]
    HostChanged = 8,  // S->C  [u8 newHostPeerId]
    RoomClosed  = 9,  // S->C  [u8 reasonCode]
    UdpToken    = 10, // S->C, TCP only  [u64 token]
    UdpPunch    = 11, // C->S, UDP only  (payload empty besides the token already in the UDP header)
    Data        = 12, // bidirectional, see header comment above
    Ping        = 13, // C->S  [u32 seq]
    Pong        = 14, // S->C  [u32 seq]  (echoed verbatim)
    Error       = 15, // S->C  [u8 errorCode][u16 len][len bytes UTF8]
};

enum class DestMode : uint8_t {
    Unicast         = 0,
    Broadcast       = 1,
    BroadcastExcept = 2,
};

enum class RoomClosedReason : uint8_t {
    HostLeft       = 0,
    Kicked         = 1,
    ServerShutdown = 2,
};

enum class ErrorCode : uint8_t {
    RoomNotFound     = 1,
    RoomFull         = 2,
    InvalidRoomCode  = 3,
    AlreadyInRoom    = 4,
    NotInRoom        = 5,
    MalformedMessage = 6,
};

// --- little-endian primitive packing into a growable byte buffer -----------

inline void PutU8(std::vector<uint8_t>& out, uint8_t v) { out.push_back(v); }

inline void PutU32(std::vector<uint8_t>& out, uint32_t v) {
    out.push_back(uint8_t(v & 0xFF));
    out.push_back(uint8_t((v >> 8) & 0xFF));
    out.push_back(uint8_t((v >> 16) & 0xFF));
    out.push_back(uint8_t((v >> 24) & 0xFF));
}

inline void PutU64(std::vector<uint8_t>& out, uint64_t v) {
    for (int i = 0; i < 8; ++i) out.push_back(uint8_t((v >> (8 * i)) & 0xFF));
}

inline void PutBytes(std::vector<uint8_t>& out, const void* data, size_t len) {
    const uint8_t* p = static_cast<const uint8_t*>(data);
    out.insert(out.end(), p, p + len);
}

// Length-prefixed (u16 LE) string — only used for the Error message text.
inline void PutLengthPrefixedString(std::vector<uint8_t>& out, const std::string& s) {
    uint16_t n = static_cast<uint16_t>(s.size());
    PutU8(out, uint8_t(n & 0xFF));
    PutU8(out, uint8_t((n >> 8) & 0xFF));
    PutBytes(out, s.data(), s.size());
}

// --- reader over a contiguous, externally-owned byte range ------------------
//
// Never throws. ok() reflects whether every read since construction (or
// since the last call that failed) stayed in-bounds — check it once after
// pulling all the fields out of a message rather than after every field.
class Reader {
public:
    Reader(const uint8_t* data, size_t len) : data_(data), len_(len) {}

    uint8_t U8() {
        if (pos_ + 1 > len_) { ok_ = false; return 0; }
        return data_[pos_++];
    }

    uint32_t U32() {
        if (pos_ + 4 > len_) { ok_ = false; return 0; }
        uint32_t v = uint32_t(data_[pos_]) | (uint32_t(data_[pos_ + 1]) << 8) |
            (uint32_t(data_[pos_ + 2]) << 16) | (uint32_t(data_[pos_ + 3]) << 24);
        pos_ += 4;
        return v;
    }

    uint64_t U64() {
        if (pos_ + 8 > len_) { ok_ = false; return 0; }
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i) v |= uint64_t(data_[pos_ + i]) << (8 * i);
        pos_ += 8;
        return v;
    }

    std::string FixedString(size_t n) {
        if (pos_ + n > len_) { ok_ = false; return {}; }
        std::string s(reinterpret_cast<const char*>(data_ + pos_), n);
        pos_ += n;
        return s;
    }

    std::string LengthPrefixedString() {
        uint16_t n = uint16_t(U8()) | (uint16_t(U8()) << 8);
        if (!ok_) return {};
        return FixedString(n);
    }

    const uint8_t* RemainingPtr() const { return data_ + pos_; }
    size_t         RemainingLen() const { return pos_ <= len_ ? len_ - pos_ : 0; }
    bool           ok()           const { return ok_; }

private:
    const uint8_t* data_;
    size_t len_;
    size_t pos_ = 0;
    bool   ok_ = true;
};

// Builds a TCP frame: [u32 length][u8 msgType][payload], appended to `out`.
inline void AppendTcpFrame(std::vector<uint8_t>& out, MessageType type,
                            const uint8_t* payload, size_t payloadLen) {
    PutU32(out, static_cast<uint32_t>(1 + payloadLen));
    PutU8(out, static_cast<uint8_t>(type));
    PutBytes(out, payload, payloadLen);
}

// Builds a C->S UDP datagram: [u64 token][u8 msgType][payload].
inline void BuildUdpDatagramToServer(std::vector<uint8_t>& out, uint64_t token,
                                      MessageType type, const uint8_t* payload, size_t payloadLen) {
    out.clear();
    PutU64(out, token);
    PutU8(out, static_cast<uint8_t>(type));
    PutBytes(out, payload, payloadLen);
}

} // namespace relay_protocol
