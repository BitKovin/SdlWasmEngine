// Vendored single-header socket library — only included here, never in the
// public header. kissnet.hpp lives in third_party/ and is checked in as-is
// (not fetched at build time) so any platform quirk found on a given
// Android NDK / FreeBSD version can be patched in place.


#ifndef __EMSCRIPTEN__

#include <kissnet.hpp>

#include "RelayTransport.h"
#include "RelayProtocol.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

using relay_protocol::MessageType;
using relay_protocol::DestMode;
using relay_protocol::RoomClosedReason;
using relay_protocol::ErrorCode;
using relay_protocol::INVALID_PEER_ID;
using relay_protocol::ROOM_CODE_LENGTH;
using Clock = std::chrono::steady_clock;

namespace {
    uint32_t MsSince(Clock::time_point t0, Clock::time_point t1) {
        return static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());
    }
}

// ---------------------------------------------------------------------------
// Impl — all kissnet types and protocol state live here
// ---------------------------------------------------------------------------

struct RelayTransport::Impl {
    static constexpr int64_t  TCP_CONNECT_TIMEOUT_MS = 5000;
    static constexpr uint32_t UDP_PUNCH_RETRY_MS      = 300;
    static constexpr int      UDP_PUNCH_MAX_RETRIES   = 5;
    static constexpr uint32_t PING_INTERVAL_MS        = 500;
    static constexpr uint32_t PING_LOST_AFTER_MS       = 2000; // unanswered ping -> counted as lost
    static constexpr size_t   RTT_SAMPLE_WINDOW       = 20;
    static constexpr size_t   MAX_TCP_RECV_CHUNK      = 4096;

    std::string relayHost;
    uint16_t    relayTcpPort = 0;
    uint16_t    relayUdpPort = 0;

    kissnet::tcp_socket tcpSocket;
    kissnet::udp_socket udpSocket;
    bool tcpConnected  = false;
    bool udpSocketOpen = false;

    bool        inRoom      = false;
    std::string roomCode;
    uint8_t     localPeerId = INVALID_PEER_ID;
    uint8_t     hostPeerId  = INVALID_PEER_ID;
    std::vector<uint8_t> activePeers; // includes self once in a room

    // UDP punch/token handshake
    uint64_t udpToken          = 0;
    bool     udpTokenReceived  = false;
    bool     udpRegistered     = false; // set once we've sent at least one punch
    int      udpPunchRetriesLeft = 0;
    Clock::time_point lastUdpPunchSentAt{};

    // TCP byte-stream buffers
    std::vector<uint8_t> tcpRecvAccum;
    std::vector<uint8_t> tcpSendQueue;
    size_t tcpSendOffset = 0;

    // Ping / RTT / loss bookkeeping. Pings go out over UDP once registered
    // (that's the channel whose quality we actually care about for
    // realtime games), falling back to TCP until then.
    uint32_t nextPingSeq = 1;
    std::unordered_map<uint32_t, Clock::time_point> outstandingPings;
    std::array<float, RTT_SAMPLE_WINDOW> rttSamplesMs{};
    size_t   rttSampleCount = 0;
    size_t   rttSampleIndex = 0;
    int      pingsSentWindow = 0;
    int      pingsLostWindow = 0;
    Clock::time_point lastPingSentAt{};

    uint64_t totalBytesIn  = 0;
    uint64_t totalBytesOut = 0;

    // ---- pending event queue ------------------------------------------------
    // Any callback (onPeerConnected, onPacketReceived, etc.) that fires while
    // the corresponding function<> is still null gets queued here instead of
    // being silently dropped. This is the common case during the wait loop
    // that polls until IsConnected() — callbacks aren't set until
    // NetworkManager::Init() runs after the loop. Without this, any Data
    // packet that arrives in the same TCP burst as RoomJoined/UdpToken is
    // dropped, causing the 9/10 "level doesn't load" failure.
    // The queue is drained at the start of the next Poll() once all three
    // required callbacks are non-null. TearDownSilent() clears it.
    struct PendingEvent {
        enum class Kind : uint8_t { PeerConnected, PeerDisconnected, HostChanged, PacketReceived } kind;
        uint8_t peerId = 0; // senderId for PacketReceived
        std::vector<uint8_t> payload; // only used for PacketReceived
    };
    std::deque<PendingEvent> pendingEvents;

    void FireOrQueue(RelayTransport& self, PendingEvent::Kind kind,
                     uint8_t peerId, const uint8_t* data = nullptr, size_t len = 0) {
        switch (kind) {
        case PendingEvent::Kind::PeerConnected:
            if (self.onPeerConnected) { self.onPeerConnected(peerId); return; }
            break;
        case PendingEvent::Kind::PeerDisconnected:
            if (self.onPeerDisconnected) { self.onPeerDisconnected(peerId); return; }
            break;
        case PendingEvent::Kind::HostChanged:
            // onHostChanged is optional — skip queueing if not set.
            if (self.onHostChanged) self.onHostChanged(peerId);
            return;
        case PendingEvent::Kind::PacketReceived:
            if (self.onPacketReceived) { self.onPacketReceived(peerId, data, len); return; }
            break;
        }
        PendingEvent ev;
        ev.kind   = kind;
        ev.peerId = peerId;
        if (data && len > 0) ev.payload.assign(data, data + len);
        pendingEvents.push_back(std::move(ev));
    }

    // Called at the start of every Poll(). Once all three required callbacks
    // are registered, replays any events that arrived before them in order.
    void DrainPendingEvents(RelayTransport& self) {
        if (pendingEvents.empty()) return;
        if (!self.onPeerConnected || !self.onPeerDisconnected || !self.onPacketReceived) return;
        while (!pendingEvents.empty()) {
            auto ev = std::move(pendingEvents.front());
            pendingEvents.pop_front();
            switch (ev.kind) {
            case PendingEvent::Kind::PeerConnected:
                self.onPeerConnected(ev.peerId);
                break;
            case PendingEvent::Kind::PeerDisconnected:
                self.onPeerDisconnected(ev.peerId);
                break;
            case PendingEvent::Kind::HostChanged:
                if (self.onHostChanged) self.onHostChanged(ev.peerId);
                break;
            case PendingEvent::Kind::PacketReceived:
                self.onPacketReceived(ev.peerId, ev.payload.data(), ev.payload.size());
                break;
            }
        }
    }

    ~Impl() { TearDownSilent(); }

    // ---- connection bootstrap ---------------------------------------------

    bool ConnectTcp() {
        // relayHost must be a bare host/IP — "127.0.0.1", not "127.0.0.1:7777".
        // kissnet's 2-argument endpoint(host, port) constructor (used below)
        // does NOT split on ':' the way its 1-argument endpoint(string)
        // overload does — it hands the host string to getaddrinfo() as-is,
        // which then fails to resolve "host:port" as a single literal.
        // Caught here with a clear message instead of letting that surface
        // as an opaque "getaddrinfo failed!" further down.
        if (relayHost.find(':') != std::string::npos) {
            std::fprintf(stderr,
                "[RelayTransport] relayHost (\"%s\") must not include a port — "
                "pass the bare host (e.g. \"127.0.0.1\") and the port separately "
                "via the RelayTransport constructor's relayTcpPort/relayUdpPort arguments.\n",
                relayHost.c_str());
            return false;
        }

        // kissnet's socket constructor calls getaddrinfo() and throws
        // std::runtime_error (via kissnet_fatal_error) on failure — e.g.
        // unresolvable host, DNS failure, no network. That constructor call
        // happens here, not inside .connect() below, so it has to be inside
        // this try as well, or a bad host crashes the caller instead of
        // CreateRoom()/JoinRoom() returning false the way the header promises.
        try {
            kissnet::endpoint ep(relayHost, relayTcpPort);
            tcpSocket = kissnet::tcp_socket(ep);
            auto status = tcpSocket.connect(TCP_CONNECT_TIMEOUT_MS);
            if (!status) {
                std::fprintf(stderr, "[RelayTransport] TCP connect to %s:%u failed\n",
                    relayHost.c_str(), relayTcpPort);
                return false;
            }
            tcpSocket.set_non_blocking(true);
            tcpSocket.set_tcp_no_delay(true);
            tcpConnected = true;

            kissnet::endpoint udpEp(relayHost, relayUdpPort);
            udpSocket = kissnet::udp_socket(udpEp);
            udpSocket.set_non_blocking(true);
            udpSocketOpen = true;
        } catch (const std::exception& ex) {
            std::fprintf(stderr, "[RelayTransport] Failed to connect to relay %s:%u — %s\n",
                relayHost.c_str(), relayTcpPort, ex.what());
            if (tcpConnected) { tcpSocket.close(); tcpConnected = false; }
            return false;
        }

        std::fprintf(stdout, "[RelayTransport] Connected to relay %s (tcp:%u udp:%u)\n",
            relayHost.c_str(), relayTcpPort, relayUdpPort);
        return true;
    }

    // Local shutdown the caller asked for (Disconnect()/LeaveRoom()) — no
    // callbacks fire, mirroring ENetTransport::Disconnect().
    void TearDownSilent() {
        if (tcpConnected)  { tcpSocket.close();  tcpConnected = false; }
        if (udpSocketOpen) { udpSocket.close();  udpSocketOpen = false; }
        inRoom = false;
        roomCode.clear();
        localPeerId = INVALID_PEER_ID;
        hostPeerId  = INVALID_PEER_ID;
        activePeers.clear();
        tcpRecvAccum.clear();
        tcpSendQueue.clear();
        tcpSendOffset = 0;
        udpToken = 0;
        udpTokenReceived = false;
        udpRegistered = false;
        udpPunchRetriesLeft = 0;
        outstandingPings.clear();
        pendingEvents.clear();
    }

    // Server-initiated or connection-loss teardown — fires onPeerDisconnected
    // for every other peer we knew about, since this is news to the caller.
    void TearDownAndNotify(RelayTransport& self) {
        for (uint8_t id : activePeers) {
            if (id == localPeerId) continue;
            if (self.onPeerDisconnected) self.onPeerDisconnected(id);
        }
        TearDownSilent();
    }

    void RemovePeer(uint8_t id) {
        activePeers.erase(std::remove(activePeers.begin(), activePeers.end(), id), activePeers.end());
    }

    // ---- TCP send/recv ------------------------------------------------------

    void QueueTcp(MessageType type, const uint8_t* payload, size_t len) {
        relay_protocol::AppendTcpFrame(tcpSendQueue, type, payload, len);
    }

    void FlushTcpSendQueue(RelayTransport& self) {
        if (!tcpConnected) return;
        while (tcpSendOffset < tcpSendQueue.size()) {
            auto result = tcpSocket.send(
                reinterpret_cast<const std::byte*>(tcpSendQueue.data() + tcpSendOffset),
                tcpSendQueue.size() - tcpSendOffset);
            size_t sent = std::get<0>(result);
            auto   status = std::get<1>(result);
            if (status == kissnet::socket_status::non_blocking_would_have_blocked) break;
            if (!status) {
                std::fprintf(stderr, "[RelayTransport] TCP send failed, disconnecting\n");
                TearDownAndNotify(self);
                return;
            }
            totalBytesOut += sent;
            tcpSendOffset += sent;
        }
        if (tcpSendOffset > 0 && tcpSendOffset == tcpSendQueue.size()) {
            tcpSendQueue.clear();
            tcpSendOffset = 0;
        }
    }

    // Parses every complete frame currently buffered and dispatches it via
    // `dispatch`. Returns false if the connection died (caller must stop).
    bool PumpTcpRecv(RelayTransport& self,
                      const std::function<void(MessageType, const uint8_t*, size_t)>& dispatch) {
        if (!tcpConnected) return false;
        std::array<std::byte, MAX_TCP_RECV_CHUNK> chunk;
        for (;;) {
            auto result = tcpSocket.recv(chunk.data(), chunk.size(), false);
            size_t n = std::get<0>(result);
            auto   status = std::get<1>(result);

            if (status == kissnet::socket_status::non_blocking_would_have_blocked) break;
            if (status == kissnet::socket_status::cleanly_disconnected) {
                std::fprintf(stdout, "[RelayTransport] Relay closed the TCP connection\n");
                TearDownAndNotify(self);
                return false;
            }

            // FIX: If we read 0 bytes, the socket is empty. On this OS, kissnet 
            // is flagging this as an error instead of 'would_have_blocked'. 
            // Break the loop and try again next Poll() tick.
            if (n == 0) {
                break;
            }

            // Only treat it as a fatal error if we actually failed while 
            // actively reading bytes, or if it's a hard OS fault.
            if (!status) {
                std::fprintf(stderr, "[RelayTransport] TCP recv error, disconnecting\n");
                TearDownAndNotify(self);
                return false;
            }

            totalBytesIn += n;
            const uint8_t* p = reinterpret_cast<const uint8_t*>(chunk.data());
            tcpRecvAccum.insert(tcpRecvAccum.end(), p, p + n);
        }

        size_t parsed = 0;
        while (tcpRecvAccum.size() - parsed >= 5) {
            relay_protocol::Reader lenReader(tcpRecvAccum.data() + parsed, 4);
            uint32_t frameLen = lenReader.U32();
            if (tcpRecvAccum.size() - parsed - 4 < frameLen) break; // wait for more bytes
            const uint8_t* framePtr = tcpRecvAccum.data() + parsed + 4;
            auto type = static_cast<MessageType>(framePtr[0]);
            dispatch(type, framePtr + 1, frameLen - 1);
            parsed += 4 + frameLen;
        }
        if (parsed > 0) tcpRecvAccum.erase(tcpRecvAccum.begin(), tcpRecvAccum.begin() + parsed);
        return true;
    }

    // ---- UDP send/recv ------------------------------------------------------

    void SendUdpToServer(MessageType type, const uint8_t* payload, size_t len) {
        if (!udpSocketOpen || !udpTokenReceived) return;
        std::vector<uint8_t> datagram;
        relay_protocol::BuildUdpDatagramToServer(datagram, udpToken, type, payload, len);
        auto result = udpSocket.send(reinterpret_cast<const std::byte*>(datagram.data()), datagram.size());
        if (std::get<1>(result)) totalBytesOut += std::get<0>(result);
    }

    void PumpUdpRecv(const std::function<void(MessageType, const uint8_t*, size_t)>& dispatch) {
        if (!udpSocketOpen) return;
        std::array<std::byte, 2048> buf;
        for (;;) {
            auto result = udpSocket.recv(buf.data(), buf.size(), false);
            size_t n      = std::get<0>(result);
            auto   status = std::get<1>(result);
            if (status == kissnet::socket_status::non_blocking_would_have_blocked) break;
            if (!status) break; // transient UDP errors are not fatal — UDP is unreliable by nature
            if (n < 1) continue;
            totalBytesIn += n;
            // S->C datagrams carry no token (see RelayProtocol.h) — just [msgType][payload]
            const uint8_t* p = reinterpret_cast<const uint8_t*>(buf.data());
            auto type = static_cast<MessageType>(p[0]);
            dispatch(type, p + 1, n - 1);
            // Any datagram at all from the relay confirms our punch registered.
            udpRegistered = true;
        }
    }

    void MaybeRetryUdpPunch() {
        if (!udpTokenReceived || udpRegistered || udpPunchRetriesLeft <= 0) return;
        auto now = Clock::now();
        if (MsSince(lastUdpPunchSentAt, now) < UDP_PUNCH_RETRY_MS) return;
        SendUdpToServer(MessageType::UdpPunch, nullptr, 0);
        lastUdpPunchSentAt = now;
        --udpPunchRetriesLeft;
    }

    // ---- ping / RTT / loss --------------------------------------------------

    void PushRttSample(float ms) {
        rttSamplesMs[rttSampleIndex] = ms;
        rttSampleIndex = (rttSampleIndex + 1) % RTT_SAMPLE_WINDOW;
        rttSampleCount = std::min(rttSampleCount + 1, RTT_SAMPLE_WINDOW);
    }

    void ProcessPingTimer() {
        if (!inRoom) return;
        auto now = Clock::now();
        if (MsSince(lastPingSentAt, now) < PING_INTERVAL_MS) return;
        lastPingSentAt = now;

        uint32_t seq = nextPingSeq++;
        std::vector<uint8_t> payload;
        relay_protocol::PutU32(payload, seq);
        if (udpRegistered) {
            SendUdpToServer(MessageType::Ping, payload.data(), payload.size());
        } else {
            QueueTcp(MessageType::Ping, payload.data(), payload.size());
        }
        outstandingPings[seq] = now;
        ++pingsSentWindow;

        // Prune pings that timed out without a Pong — counted as lost.
        for (auto it = outstandingPings.begin(); it != outstandingPings.end();) {
            if (MsSince(it->second, now) > PING_LOST_AFTER_MS) {
                ++pingsLostWindow;
                it = outstandingPings.erase(it);
            } else {
                ++it;
            }
        }
        // Keep the loss window from growing unbounded over a long session.
        if (pingsSentWindow >= 50) {
            pingsSentWindow /= 2;
            pingsLostWindow /= 2;
        }
    }

    void HandlePong(const uint8_t* payload, size_t len) {
        relay_protocol::Reader r(payload, len);
        uint32_t seq = r.U32();
        if (!r.ok()) return;
        auto it = outstandingPings.find(seq);
        if (it == outstandingPings.end()) return; // already timed out and counted as lost
        float ms = static_cast<float>(MsSince(it->second, Clock::now()));
        PushRttSample(ms);
        outstandingPings.erase(it);
    }

    // ---- room control message dispatch --------------------------------------

    void HandleMessage(RelayTransport& self, MessageType type, const uint8_t* payload, size_t len) {
        relay_protocol::Reader r(payload, len);
        switch (type) {
        case MessageType::RoomCreated: {
            roomCode = r.FixedString(ROOM_CODE_LENGTH);
            uint8_t peerId = r.U8();
            if (!r.ok()) return;
            localPeerId = peerId;
            hostPeerId  = peerId; // always 0
            inRoom = true;
            activePeers = { peerId };
            std::fprintf(stdout, "[RelayTransport] Room created: %s (peerId=%u)\n",
                roomCode.c_str(), peerId);
            FireOrQueue(self, PendingEvent::Kind::PeerConnected, peerId);
            break;
        }
        case MessageType::RoomJoined: {
            roomCode = r.FixedString(ROOM_CODE_LENGTH);
            uint8_t assignedId = r.U8();
            uint8_t host       = r.U8();
            uint8_t peerCount  = r.U8();
            std::vector<uint8_t> roster;
            for (uint8_t i = 0; i < peerCount && r.ok(); ++i) roster.push_back(r.U8());
            if (!r.ok()) return;
            localPeerId = assignedId;
            hostPeerId  = host;
            inRoom = true;
            activePeers = roster;
            if (std::find(activePeers.begin(), activePeers.end(), assignedId) == activePeers.end())
                activePeers.push_back(assignedId);
            std::fprintf(stdout, "[RelayTransport] Joined room (peerId=%u, host=%u, %u existing peers)\n",
                assignedId, host, peerCount);
            FireOrQueue(self, PendingEvent::Kind::PeerConnected, assignedId);
            for (uint8_t id : roster) {
                if (id != assignedId)
                    FireOrQueue(self, PendingEvent::Kind::PeerConnected, id);
            }
            break;
        }
        case MessageType::PeerJoined: {
            uint8_t id = r.U8();
            if (!r.ok()) return;
            if (std::find(activePeers.begin(), activePeers.end(), id) == activePeers.end())
                activePeers.push_back(id);
            FireOrQueue(self, PendingEvent::Kind::PeerConnected, id);
            break;
        }
        case MessageType::PeerLeft: {
            uint8_t id = r.U8();
            if (!r.ok()) return;
            RemovePeer(id);
            FireOrQueue(self, PendingEvent::Kind::PeerDisconnected, id);
            break;
        }
        case MessageType::HostChanged: {
            uint8_t newHost = r.U8();
            if (!r.ok()) return;
            hostPeerId = newHost;
            FireOrQueue(self, PendingEvent::Kind::HostChanged, newHost);
            break;
        }
        case MessageType::RoomClosed: {
            uint8_t reason = r.U8();
            std::fprintf(stdout, "[RelayTransport] Room closed by relay (reason=%u)\n",
                r.ok() ? reason : 0xFF);
            TearDownAndNotify(self);
            break;
        }
        case MessageType::UdpToken: {
            uint64_t token = r.U64();
            if (!r.ok()) return;
            udpToken = token;
            udpTokenReceived = true;
            udpPunchRetriesLeft = UDP_PUNCH_MAX_RETRIES;
            SendUdpToServer(MessageType::UdpPunch, nullptr, 0);
            lastUdpPunchSentAt = Clock::now();
            --udpPunchRetriesLeft;
            break;
        }
        case MessageType::Pong:
            HandlePong(payload, len);
            break;
        case MessageType::Data: {
            uint8_t senderId = r.U8();
            if (!r.ok()) return;
            FireOrQueue(self, PendingEvent::Kind::PacketReceived,
                senderId, r.RemainingPtr(), r.RemainingLen());
            break;
        }
        case MessageType::Error: {
            uint8_t code = r.U8();
            std::string msg = r.LengthPrefixedString();
            std::fprintf(stderr, "[RelayTransport] Relay error %u: %s\n",
                code, msg.c_str());
            break;
        }
        default:
            break;
        }
    }
};

// ---------------------------------------------------------------------------
// RelayTransport
// ---------------------------------------------------------------------------

RelayTransport::RelayTransport(std::string relayHost, uint16_t relayTcpPort, uint16_t relayUdpPort)
    : impl_(std::make_unique<Impl>()) {
    impl_->relayHost    = std::move(relayHost);
    impl_->relayTcpPort = relayTcpPort;
    impl_->relayUdpPort = relayUdpPort;
}

RelayTransport::~RelayTransport() {
    Disconnect();
}

bool RelayTransport::Host(uint16_t /*port*/, int maxClients) {
    return CreateRoom(maxClients, /*closeRoomOnHostLeave=*/false);
}

bool RelayTransport::Connect(const std::string& address, uint16_t /*port*/) {
    return JoinRoom(address);
}

bool RelayTransport::CreateRoom(int maxClients, bool closeRoomOnHostLeave) {
    assert(!impl_->tcpConnected && "CreateRoom called while already connected");
    if (maxClients < 1 || maxClients > 255) {
        std::fprintf(stderr, "[RelayTransport] CreateRoom: maxClients must be 1..255\n");
        return false;
    }
    if (!impl_->ConnectTcp()) return false;

    std::vector<uint8_t> payload;
    relay_protocol::PutU8(payload, static_cast<uint8_t>(maxClients));
    relay_protocol::PutU8(payload, closeRoomOnHostLeave ? 1 : 0);
    impl_->QueueTcp(MessageType::CreateRoom, payload.data(), payload.size());
    impl_->FlushTcpSendQueue(*this);
    return true;
}

bool RelayTransport::JoinRoom(const std::string& roomCode) {
    assert(!impl_->tcpConnected && "JoinRoom called while already connected");
    if (roomCode.size() != ROOM_CODE_LENGTH) {
        std::fprintf(stderr, "[RelayTransport] JoinRoom: room code must be %zu characters\n",
            ROOM_CODE_LENGTH);
        return false;
    }
    if (!impl_->ConnectTcp()) return false;

    impl_->QueueTcp(MessageType::JoinRoom,
        reinterpret_cast<const uint8_t*>(roomCode.data()), roomCode.size());
    impl_->FlushTcpSendQueue(*this);
    return true;
}

void RelayTransport::LeaveRoom() {
    if (impl_->tcpConnected && impl_->inRoom) {
        impl_->QueueTcp(MessageType::LeaveRoom, nullptr, 0);
        impl_->FlushTcpSendQueue(*this);
    }
    impl_->TearDownSilent();
}

void RelayTransport::Disconnect() {
    if (impl_->tcpConnected && impl_->inRoom) {
        impl_->QueueTcp(MessageType::LeaveRoom, nullptr, 0);
        impl_->FlushTcpSendQueue(*this);
    }
    impl_->TearDownSilent();
}

void RelayTransport::Poll() {
    if (!impl_->tcpConnected) return;

    // Deliver any events (PeerConnected, PacketReceived, etc.) that arrived
    // before NetworkManager::Init() registered the callbacks. This is the
    // fix for the "level doesn't load 9/10 times" race: LevelInfo can
    // arrive in the same TCP burst as RoomJoined before Init() runs.
    impl_->DrainPendingEvents(*this);

    auto dispatch = [this](MessageType type, const uint8_t* payload, size_t len) {
        impl_->HandleMessage(*this, type, payload, len);
    };

    if (!impl_->PumpTcpRecv(*this, dispatch)) return; // connection died
    impl_->FlushTcpSendQueue(*this);
    if (!impl_->tcpConnected) return; // FlushTcpSendQueue may have torn down on error

    impl_->PumpUdpRecv(dispatch);
    impl_->MaybeRetryUdpPunch();
    impl_->ProcessPingTimer();
}

namespace {
    void AppendDataHeader(std::vector<uint8_t>& out, DestMode mode, uint8_t destOrExclude) {
        relay_protocol::PutU8(out, static_cast<uint8_t>(mode));
        relay_protocol::PutU8(out, destOrExclude);
    }
}

void RelayTransport::Send(uint8_t peerId, const uint8_t* data, size_t length, bool reliable) {
    if (!impl_->inRoom) return;
    std::vector<uint8_t> payload;
    AppendDataHeader(payload, DestMode::Unicast, peerId);
    relay_protocol::PutBytes(payload, data, length);

    if (reliable) {
        impl_->QueueTcp(MessageType::Data, payload.data(), payload.size());
    } else {
        impl_->SendUdpToServer(MessageType::Data, payload.data(), payload.size());
    }
}

void RelayTransport::Broadcast(const uint8_t* data, size_t length, bool reliable) {
    if (!impl_->inRoom) return;
    std::vector<uint8_t> payload;
    AppendDataHeader(payload, DestMode::Broadcast, 0);
    relay_protocol::PutBytes(payload, data, length);

    if (reliable) {
        impl_->QueueTcp(MessageType::Data, payload.data(), payload.size());
    } else {
        impl_->SendUdpToServer(MessageType::Data, payload.data(), payload.size());
    }
}

void RelayTransport::BroadcastExcept(uint8_t excludePeerId,
    const uint8_t* data, size_t length, bool reliable) {
    if (!impl_->inRoom) return;
    std::vector<uint8_t> payload;
    AppendDataHeader(payload, DestMode::BroadcastExcept, excludePeerId);
    relay_protocol::PutBytes(payload, data, length);

    if (reliable) {
        impl_->QueueTcp(MessageType::Data, payload.data(), payload.size());
    } else {
        impl_->SendUdpToServer(MessageType::Data, payload.data(), payload.size());
    }
}

bool RelayTransport::IsConnected() const {
    return impl_->tcpConnected && impl_->inRoom;
}

int RelayTransport::GetPeerCount() const {
    return static_cast<int>(impl_->activePeers.size());
}

std::string RelayTransport::GetConnectionString() const {
    return impl_->roomCode;
}

uint8_t RelayTransport::GetLocalPeerId() const { return impl_->localPeerId; }
uint8_t RelayTransport::GetHostPeerId()  const { return impl_->hostPeerId; }

NetworkStat RelayTransport::GetStat() const {
    NetworkStat stat{};
    stat.incomingBytesTotal = impl_->totalBytesIn;
    stat.outgoingBytesTotal = impl_->totalBytesOut;

    // RTT/loss measure THIS client's link to the relay — not end-to-end to
    // another peer. See the note in RelayTransport.h / the architecture doc.
    if (impl_->rttSampleCount > 0) {
        double sum = 0.0, sumSq = 0.0;
        for (size_t i = 0; i < impl_->rttSampleCount; ++i) {
            sum += impl_->rttSamplesMs[i];
            sumSq += double(impl_->rttSamplesMs[i]) * double(impl_->rttSamplesMs[i]);
        }
        double mean = sum / impl_->rttSampleCount;
        double variance = impl_->rttSampleCount > 1
            ? (sumSq / impl_->rttSampleCount) - (mean * mean)
            : 0.0;
        stat.roundTripTime         = static_cast<uint16_t>(std::min(mean, double(UINT16_MAX)));
        stat.roundTripTimeVariance = static_cast<uint16_t>(std::min(std::max(variance, 0.0), double(UINT16_MAX)));
    }

    if (impl_->pingsSentWindow > 0) {
        float loss = float(impl_->pingsLostWindow) / float(impl_->pingsSentWindow) * 100.0f;
        stat.packetLossPercent = loss;
        stat.packetLossPercentVariance = 0.0f; // single rolling estimate, no variance tracked
    }

    return stat;
}

#endif // !__EMSCRIPTEN__