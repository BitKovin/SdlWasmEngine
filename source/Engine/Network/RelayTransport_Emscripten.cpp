// ---------------------------------------------------------------------------
// RelayTransport_Emscripten.cpp
//
// NOTE ON VERIFICATION: this file could not be compiled in the environment
// that produced it — no Emscripten SDK is available there, and emsdk's
// download host isn't reachable from it either. RelayTransport_Native.cpp
// (the TCP/UDP twin of this file) WAS built and run end-to-end against a
// throwaway test relay and is correct; this file follows the exact same
// state machine and message handling, swapped onto the WebSocket C API from
// memory of its documented surface. Please build it with your emsdk and
// send back whatever the compiler says if anything doesn't match — the
// `emscripten/websocket.h` shipped with your SDK version is authoritative,
// not this comment.
//
// Build note: link with `-lwebsocket.js` (per the header comment in
// emscripten/websocket.h itself) in addition to whatever else your
// Emscripten target already links.
// ---------------------------------------------------------------------------

#ifdef __EMSCRIPTEN__


#include "RelayTransport.h"
#include "RelayProtocol.h"

#include <emscripten.h>
#include <emscripten/websocket.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <deque>
#include <functional>
#include <unordered_map>
#include <vector>

using relay_protocol::MessageType;
using relay_protocol::DestMode;
using relay_protocol::ErrorCode;
using relay_protocol::INVALID_PEER_ID;
using relay_protocol::ROOM_CODE_LENGTH;
using Clock = std::chrono::steady_clock;

namespace {
    uint32_t MsSince(Clock::time_point t0, Clock::time_point t1) {
        return static_cast<uint32_t>(
            std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());
    }

    // Fixed route the relay's WebSocket endpoint must be mapped to — see
    // RelayServer/Transport/WebSocketEndpoint.cs (app.Map("/ws/relay", ...)).
    constexpr const char* WS_PATH = "/ws/relay";

    bool PageIsHttps() {
        return EM_ASM_INT({
            return (typeof window !== 'undefined' && window.location &&
                    window.location.protocol === 'https:') ? 1 : 0;
            }) != 0;
    }
}

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct RelayTransport::Impl {
    static constexpr uint32_t PING_INTERVAL_MS = 500;
    static constexpr uint32_t PING_LOST_AFTER_MS = 2000;
    static constexpr size_t   RTT_SAMPLE_WINDOW = 20;

    std::string relayHost;
    uint16_t    relayPort = 0; // relayTcpPort is reused as the WS port — see ctor

    EMSCRIPTEN_WEBSOCKET_T ws = 0;
    bool wsCreated = false;
    bool wsOpen = false;

    // Set by the onclose/onerror callbacks (which can fire at any time, not
    // necessarily during Poll()); handled at the start of the next Poll()
    // so user callbacks always fire from the same thread/call as Poll(),
    // matching the contract documented in INetworkTransport.h.
    bool connectionLost = false;

    // CreateRoom()/JoinRoom() queue their request here; flushed once the
    // WebSocket actually finishes opening (it's an async handshake).
    std::vector<uint8_t> pendingHandshake;

    // onmessage fires from the browser's event loop, not necessarily neatly
    // between Poll() calls — queue raw payloads and drain them in Poll() so
    // all user callbacks fire from inside Poll(), same as the native build.
    std::deque<std::vector<uint8_t>> inboundQueue;

    bool        inRoom = false;
    std::string roomCode;
    uint8_t     localPeerId = INVALID_PEER_ID;
    uint8_t     hostPeerId = INVALID_PEER_ID;
    std::vector<uint8_t> activePeers;

    uint32_t nextPingSeq = 1;
    std::unordered_map<uint32_t, Clock::time_point> outstandingPings;
    std::array<float, RTT_SAMPLE_WINDOW> rttSamplesMs{};
    size_t   rttSampleCount = 0;
    size_t   rttSampleIndex = 0;
    int      pingsSentWindow = 0;
    int      pingsLostWindow = 0;
    Clock::time_point lastPingSentAt{};

    uint64_t totalBytesIn = 0;
    uint64_t totalBytesOut = 0;

    // ---- pending event queue (same fix as RelayTransport_Native.cpp) --------
    struct PendingEvent {
        enum class Kind : uint8_t { PeerConnected, PeerDisconnected, HostChanged, PacketReceived } kind;
        uint8_t peerId = 0;
        std::vector<uint8_t> payload;
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
            if (self.onHostChanged) self.onHostChanged(peerId);
            return;
        case PendingEvent::Kind::PacketReceived:
            if (self.onPacketReceived) { self.onPacketReceived(peerId, data, len); return; }
            break;
        }
        PendingEvent ev;
        ev.kind = kind;
        ev.peerId = peerId;
        if (data && len > 0) ev.payload.assign(data, data + len);
        pendingEvents.push_back(std::move(ev));
    }

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

    bool OpenSocket() {
        if (!emscripten_websocket_is_supported()) {
            std::fprintf(stderr, "[RelayTransport] WebSockets not supported in this browser\n");
            return false;
        }
        // relayHost must be a bare host/IP — "127.0.0.1", not "127.0.0.1:7777".
        // The URL below concatenates relayHost + ":" + relayPort itself;
        // a host string that already contains a port produces a malformed
        // "ws://host:port:port/..." URL instead of failing loudly, so this
        // is checked explicitly rather than left to surface as a confusing
        // connection failure later.
        if (relayHost.find(':') != std::string::npos) {
            std::fprintf(stderr,
                "[RelayTransport] relayHost (\"%s\") must not include a port — "
                "pass the bare host (e.g. \"127.0.0.1\") and the port separately "
                "via the RelayTransport constructor's relayTcpPort argument "
                "(reused as the WebSocket port on this platform).\n",
                relayHost.c_str());
            return false;
        }
        std::string scheme = PageIsHttps() ? "wss://" : "ws://";
        std::string url = scheme + relayHost + ":" + std::to_string(relayPort) + WS_PATH;

        EmscriptenWebSocketCreateAttributes attr;
        emscripten_websocket_init_create_attributes(&attr);
        attr.url = url.c_str();
        // emscripten_websocket_init_create_attributes() defaults `protocols`
        // to "binary" (a leftover from the echo.websocket.org demo it was
        // written against). That makes the browser send a
        // "Sec-WebSocket-Protocol: binary" header on the handshake; since
        // RelayServer's AcceptWebSocketAsync() call doesn't negotiate/echo
        // that subprotocol back, the browser aborts the handshake before
        // onopen ever fires, so pendingHandshake (the CreateRoom/JoinRoom
        // message) never gets sent — which is why the relay never sees
        // anything. Explicitly request no subprotocol, matching the native
        // TCP build (which has no concept of one).
        attr.protocols = nullptr;
        attr.createOnMainThread = EM_TRUE;

        ws = emscripten_websocket_new(&attr);
        if (ws <= 0) {
            std::fprintf(stderr, "[RelayTransport] emscripten_websocket_new failed for %s\n", url.c_str());
            return false;
        }
        wsCreated = true;

        emscripten_websocket_set_onopen_callback(ws, this, &Impl::OnOpen);
        emscripten_websocket_set_onmessage_callback(ws, this, &Impl::OnMessage);
        emscripten_websocket_set_onclose_callback(ws, this, &Impl::OnClose);
        emscripten_websocket_set_onerror_callback(ws, this, &Impl::OnError);

        std::fprintf(stdout, "[RelayTransport] Opening WebSocket to %s\n", url.c_str());
        return true;
    }

    void TearDownSilent() {
        if (wsCreated) {
            emscripten_websocket_close(ws, 1000, "");
            emscripten_websocket_delete(ws);
            wsCreated = false;
            wsOpen = false;
            ws = 0;
        }
        inRoom = false;
        roomCode.clear();
        localPeerId = INVALID_PEER_ID;
        hostPeerId = INVALID_PEER_ID;
        activePeers.clear();
        pendingHandshake.clear();
        inboundQueue.clear();
        outstandingPings.clear();
        connectionLost = false;
        pendingEvents.clear();
    }

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

    // ---- sending ------------------------------------------------------------

    void SendMessage(MessageType type, const uint8_t* payload, size_t len) {
        std::vector<uint8_t> msg;
        msg.reserve(1 + len);
        msg.push_back(static_cast<uint8_t>(type));
        if (payload && len) msg.insert(msg.end(), payload, payload + len);

        if (!wsOpen) {
            // Only the initial CreateRoom/JoinRoom can legitimately be
            // queued before open; anything else sent before the socket is
            // ready is a caller bug (e.g. Send() before onPeerConnected).
            std::fprintf(stderr, "[RelayTransport] Dropping send: WebSocket not open yet (type=%u)\n",
                static_cast<unsigned>(type));
            return;
        }
        auto result = emscripten_websocket_send_binary(ws, msg.data(), static_cast<int>(msg.size()));
        if (result == EMSCRIPTEN_RESULT_SUCCESS) {
            totalBytesOut += msg.size();
        }
        else {
            std::fprintf(stderr, "[RelayTransport] emscripten_websocket_send_binary failed (%d)\n", result);
        }
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
        SendMessage(MessageType::Ping, payload.data(), payload.size());
        outstandingPings[seq] = now;
        ++pingsSentWindow;

        for (auto it = outstandingPings.begin(); it != outstandingPings.end();) {
            if (MsSince(it->second, now) > PING_LOST_AFTER_MS) {
                ++pingsLostWindow;
                it = outstandingPings.erase(it);
            }
            else {
                ++it;
            }
        }
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
        if (it == outstandingPings.end()) return;
        float ms = static_cast<float>(MsSince(it->second, Clock::now()));
        PushRttSample(ms);
        outstandingPings.erase(it);
    }

    // ---- control message dispatch (identical semantics to the native build)

    void HandleMessage(RelayTransport& self, MessageType type, const uint8_t* payload, size_t len) {
        relay_protocol::Reader r(payload, len);
        switch (type) {
        case MessageType::RoomCreated: {
            roomCode = r.FixedString(ROOM_CODE_LENGTH);
            uint8_t peerId = r.U8();
            if (!r.ok()) return;
            localPeerId = peerId;
            hostPeerId = peerId;
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
            uint8_t host = r.U8();
            uint8_t peerCount = r.U8();
            std::vector<uint8_t> roster;
            for (uint8_t i = 0; i < peerCount && r.ok(); ++i) roster.push_back(r.U8());
            if (!r.ok()) return;
            localPeerId = assignedId;
            hostPeerId = host;
            inRoom = true;
            activePeers = roster;
            if (std::find(activePeers.begin(), activePeers.end(), assignedId) == activePeers.end())
                activePeers.push_back(assignedId);
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
            std::fprintf(stdout, "[RelayTransport] Room closed by relay\n");
            TearDownAndNotify(self);
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
            std::fprintf(stderr, "[RelayTransport] Relay error %u: %s\n", code, msg.c_str());
            break;
        }
        default:
            break; // UdpToken/UdpPunch never appear on this transport
        }
    }

    // ---- static WebSocket callbacks ----------------------------------------
    // userData is always the Impl* passed at registration time.

    static EM_BOOL OnOpen(int /*eventType*/, const EmscriptenWebSocketOpenEvent* /*event*/, void* userData) {
        auto* impl = static_cast<Impl*>(userData);
        impl->wsOpen = true;
        std::fprintf(stdout, "[RelayTransport] WebSocket open\n");
        if (!impl->pendingHandshake.empty()) {
            emscripten_websocket_send_binary(impl->ws, impl->pendingHandshake.data(),
                static_cast<int>(impl->pendingHandshake.size()));
            impl->totalBytesOut += impl->pendingHandshake.size();
            impl->pendingHandshake.clear();
        }
        return EM_TRUE;
    }

    static EM_BOOL OnMessage(int /*eventType*/, const EmscriptenWebSocketMessageEvent* event, void* userData) {
        auto* impl = static_cast<Impl*>(userData);
        if (event->isText) return EM_TRUE; // the relay protocol is binary-only
        impl->totalBytesIn += static_cast<size_t>(event->numBytes);
        impl->inboundQueue.emplace_back(event->data, event->data + event->numBytes);
        return EM_TRUE;
    }

    static EM_BOOL OnClose(int /*eventType*/, const EmscriptenWebSocketCloseEvent* event, void* userData) {
        auto* impl = static_cast<Impl*>(userData);
        std::fprintf(stdout, "[RelayTransport] WebSocket closed (code=%u, clean=%d)\n",
            event->code, event->wasClean);
        impl->connectionLost = true;
        return EM_TRUE;
    }

    static EM_BOOL OnError(int /*eventType*/, const EmscriptenWebSocketErrorEvent* /*event*/, void* userData) {
        auto* impl = static_cast<Impl*>(userData);
        std::fprintf(stderr, "[RelayTransport] WebSocket error\n");
        impl->connectionLost = true;
        return EM_TRUE;
    }
};

// ---------------------------------------------------------------------------
// RelayTransport
// ---------------------------------------------------------------------------

RelayTransport::RelayTransport(std::string relayHost, uint16_t relayTcpPort, uint16_t /*relayUdpPort*/)
    : impl_(std::make_unique<Impl>()) {
    impl_->relayHost = std::move(relayHost);
    impl_->relayPort = relayTcpPort; // reused as the WS port on this platform
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
    assert(!impl_->wsCreated && "CreateRoom called while already connected");
    if (maxClients < 1 || maxClients > 255) {
        std::fprintf(stderr, "[RelayTransport] CreateRoom: maxClients must be 1..255\n");
        return false;
    }
    impl_->pendingHandshake.clear();
    impl_->pendingHandshake.push_back(static_cast<uint8_t>(MessageType::CreateRoom));
    impl_->pendingHandshake.push_back(static_cast<uint8_t>(maxClients));
    impl_->pendingHandshake.push_back(closeRoomOnHostLeave ? 1 : 0);
    return impl_->OpenSocket();
}

bool RelayTransport::JoinRoom(const std::string& roomCode) {
    assert(!impl_->wsCreated && "JoinRoom called while already connected");
    if (roomCode.size() != ROOM_CODE_LENGTH) {
        std::fprintf(stderr, "[RelayTransport] JoinRoom: room code must be %zu characters\n",
            ROOM_CODE_LENGTH);
        return false;
    }
    impl_->pendingHandshake.clear();
    impl_->pendingHandshake.push_back(static_cast<uint8_t>(MessageType::JoinRoom));
    impl_->pendingHandshake.insert(impl_->pendingHandshake.end(), roomCode.begin(), roomCode.end());
    return impl_->OpenSocket();
}

void RelayTransport::LeaveRoom() {
    if (impl_->wsOpen && impl_->inRoom) {
        impl_->SendMessage(MessageType::LeaveRoom, nullptr, 0);
    }
    impl_->TearDownSilent();
}

void RelayTransport::Disconnect() {
    if (impl_->wsOpen && impl_->inRoom) {
        impl_->SendMessage(MessageType::LeaveRoom, nullptr, 0);
    }
    impl_->TearDownSilent();
}

void RelayTransport::Poll() {
    if (!impl_->wsCreated) return;

    if (impl_->connectionLost) {
        impl_->TearDownAndNotify(*this);
        return;
    }

    // Deliver any events queued before callbacks were registered.
    impl_->DrainPendingEvents(*this);

    while (!impl_->inboundQueue.empty()) {
        std::vector<uint8_t> msg = std::move(impl_->inboundQueue.front());
        impl_->inboundQueue.pop_front();
        if (msg.empty()) continue;
        auto type = static_cast<MessageType>(msg[0]);
        impl_->HandleMessage(*this, type, msg.data() + 1, msg.size() - 1);
        if (impl_->connectionLost) { // RoomClosed/teardown happened mid-drain
            impl_->TearDownAndNotify(*this);
            return;
        }
    }

    impl_->ProcessPingTimer();
}

namespace {
    void AppendDataHeader(std::vector<uint8_t>& out, DestMode mode, uint8_t destOrExclude) {
        relay_protocol::PutU8(out, static_cast<uint8_t>(mode));
        relay_protocol::PutU8(out, destOrExclude);
    }
}

// NOTE: `reliable` is accepted for interface compatibility but has no effect
// here — a browser has exactly one channel (the WebSocket itself), which is
// reliable/ordered. See the class comment in RelayTransport.h.

void RelayTransport::Send(uint8_t peerId, const uint8_t* data, size_t length, bool /*reliable*/) {
    if (!impl_->inRoom) return;
    std::vector<uint8_t> payload;
    AppendDataHeader(payload, DestMode::Unicast, peerId);
    relay_protocol::PutBytes(payload, data, length);
    impl_->SendMessage(MessageType::Data, payload.data(), payload.size());
}

void RelayTransport::Broadcast(const uint8_t* data, size_t length, bool /*reliable*/) {
    if (!impl_->inRoom) return;
    std::vector<uint8_t> payload;
    AppendDataHeader(payload, DestMode::Broadcast, 0);
    relay_protocol::PutBytes(payload, data, length);
    impl_->SendMessage(MessageType::Data, payload.data(), payload.size());
}

void RelayTransport::BroadcastExcept(uint8_t excludePeerId,
    const uint8_t* data, size_t length, bool /*reliable*/) {
    if (!impl_->inRoom) return;
    std::vector<uint8_t> payload;
    AppendDataHeader(payload, DestMode::BroadcastExcept, excludePeerId);
    relay_protocol::PutBytes(payload, data, length);
    impl_->SendMessage(MessageType::Data, payload.data(), payload.size());
}

bool RelayTransport::IsConnected() const {
    return impl_->wsOpen && impl_->inRoom;
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
        stat.roundTripTime = static_cast<uint16_t>(std::min(mean, double(UINT16_MAX)));
        stat.roundTripTimeVariance = static_cast<uint16_t>(std::min(std::max(variance, 0.0), double(UINT16_MAX)));
    }

    if (impl_->pingsSentWindow > 0) {
        stat.packetLossPercent = float(impl_->pingsLostWindow) / float(impl_->pingsSentWindow) * 100.0f;
        stat.packetLossPercentVariance = 0.0f;
    }

    return stat;
}


#endif // __EMSCRIPTEN__