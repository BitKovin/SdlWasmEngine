// ENet and Windows headers are included only here, never in the header.
#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#  define NOMINMAX
#endif
#define _HAS_STD_BYTE 0
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// ENET_IMPLEMENTATION must be defined in exactly one .cpp file.
// It causes enet.h to emit all function definitions (single-header style).
#define ENET_IMPLEMENTATION
#include <enet/enet.h>

#include "ENetTransport.h"

#include <cassert>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <unordered_map>

// ---------------------------------------------------------------------------
// ENet global ref-count
// ---------------------------------------------------------------------------

namespace {

    int g_enetRefCount = 0;

    void ENetAddRef() {
        if (g_enetRefCount == 0) {
            if (enet_initialize() != 0) {
                std::fprintf(stderr, "[ENetTransport] enet_initialize() failed\n");
                std::abort();
            }
        }
        ++g_enetRefCount;
    }

    void ENetRelease() {
        assert(g_enetRefCount > 0);
        if (--g_enetRefCount == 0) {
            enet_deinitialize();
        }
    }

} // namespace

// ---------------------------------------------------------------------------
// Impl — all ENet types live here
// ---------------------------------------------------------------------------

struct ENetTransport::Impl {
    static constexpr enet_uint8  CHANNEL_RELIABLE = 0;
    static constexpr enet_uint8  CHANNEL_UNRELIABLE = 1;
    static constexpr int         CHANNEL_COUNT = 2;

    // How often each peer is pinged (ms).  Tighter than the ENet default
    // (500 ms) so that RTT estimates converge faster and timeout detection
    // fires on the first missed window rather than after several slow cycles.
    static constexpr enet_uint32 PEER_PING_INTERVAL_MS = 50;

    // Peer timeout window.  If no data (including pings/ACKs) is received
    // from a peer for longer than PEER_TIMEOUT_MIN_MS, ENet begins the
    // timeout process.  If silence continues past PEER_TIMEOUT_MAX_MS the
    // peer is forcibly disconnected and ENET_EVENT_TYPE_DISCONNECT fires.
    //
    // ENet defaults are 5 000 ms / 30 000 ms — far too long for development
    // where clients routinely pause on breakpoints.  With the values below a
    // frozen client is evicted within 1.5–4 s.
    static constexpr enet_uint32 PEER_TIMEOUT_LIMIT_MS = ENET_PEER_TIMEOUT_LIMIT; // keep default multiplier (32)
    static constexpr enet_uint32 PEER_TIMEOUT_MIN_MS = 5000;
    static constexpr enet_uint32 PEER_TIMEOUT_MAX_MS = 15000;

    ENetHost* host = nullptr;
    bool      isServer = false;
    uint8_t   nextPeerId = 1;

    // Backing storage for GetConnectionString(): "address:port" for Connect(),
    // "0.0.0.0:port" for Host(). Cleared on Disconnect().
    std::string connectionDesc;

    ENetPeer* serverPeer = nullptr; // client-only

    std::unordered_map<uint8_t, ENetPeer*> peerById;
    std::unordered_map<ENetPeer*, uint8_t>   idByPeer;

    // ── Helpers ──────────────────────────────────────────────────────────

    ENetPeer* PeerForId(uint8_t peerId) const {
        auto it = peerById.find(peerId);

        return it != peerById.end() ? it->second : nullptr;
    }

    uint8_t IdForPeer(ENetPeer* peer) const {
        auto it = idByPeer.find(peer);
        if (it != idByPeer.end()) return it->second;
        return static_cast<uint8_t>(reinterpret_cast<uintptr_t>(peer->data));
    }

    enet_uint32 SendFlags(bool reliable) const {
        return reliable ? ENET_PACKET_FLAG_RELIABLE : 0;
    }

    enet_uint8 Channel(bool reliable) const {
        return reliable ? CHANNEL_RELIABLE : CHANNEL_UNRELIABLE;
    }

    // ── Event handlers ───────────────────────────────────────────────────

    void HandleConnect(ENetEvent& event,
        const std::function<void(uint8_t)>& onConnected) {

        // Tighten the ping cadence so RTT estimates converge quickly.
        enet_peer_ping_interval(event.peer, PEER_PING_INTERVAL_MS);

        // Set an aggressive timeout so that a peer that stops responding
        // (e.g. a client frozen on a breakpoint) is evicted within a few
        // seconds instead of ENet's default 5–30 s window.
        //
        // Note: enet_peer_ping_interval only affects RTT sampling; it has
        // NO effect on when ENet declares a peer dead.  enet_peer_timeout is
        // the only knob that controls that threshold.
        enet_peer_timeout(event.peer,
            PEER_TIMEOUT_LIMIT_MS,
            PEER_TIMEOUT_MIN_MS,
            PEER_TIMEOUT_MAX_MS);

        if (isServer) {
            if (nextPeerId == 0) {
                std::fprintf(stderr, "[ENetTransport] Out of peer IDs\n");
                enet_peer_disconnect(event.peer, 0);
                return;
            }
            uint8_t newId = nextPeerId++;
            peerById[newId] = event.peer;
            idByPeer[event.peer] = newId;
            event.peer->data = reinterpret_cast<void*>(
                static_cast<uintptr_t>(newId));
            std::fprintf(stdout, "[ENetTransport] Client connected, peerId=%u\n", newId);
            if (onConnected) onConnected(newId);
        }
        else {
            peerById[0] = event.peer;
            idByPeer[event.peer] = 0;
            event.peer->data = reinterpret_cast<void*>(0);
            std::fprintf(stdout, "[ENetTransport] Connected to server\n");
            if (onConnected) onConnected(0);
        }
    }

    void HandleDisconnect(ENetEvent& event,
        const std::function<void(uint8_t)>& onDisconnected) {
        uint8_t peerId = IdForPeer(event.peer);
        std::fprintf(stdout, "[ENetTransport] Peer %u disconnected\n", peerId);
        peerById.erase(peerId);
        idByPeer.erase(event.peer);
        if (!isServer && event.peer == serverPeer) serverPeer = nullptr;
        if (onDisconnected) onDisconnected(peerId);
    }

    void HandleReceive(ENetEvent& event,
        const std::function<void(uint8_t, const uint8_t*, size_t)>& onReceived) {
        uint8_t senderId = IdForPeer(event.peer);
        if (onReceived)
            onReceived(senderId, event.packet->data, event.packet->dataLength);
        enet_packet_destroy(event.packet);
    }

    // Clean up a failed client connection attempt.
    // Uses disconnect_now + flush so the socket is released before the
    // caller tries to bind the same port as a server host.
    void TeardownClientHost() {
        if (serverPeer) {
            enet_peer_disconnect_now(serverPeer, 0);
            serverPeer = nullptr;
        }
        if (host) {
            enet_host_flush(host);
            enet_host_destroy(host);
            host = nullptr;
        }
        peerById.clear();
        idByPeer.clear();
    }
};

// ---------------------------------------------------------------------------
// ENetTransport
// ---------------------------------------------------------------------------

ENetTransport::ENetTransport()
    : impl_(std::make_unique<Impl>()) {
    ENetAddRef();
}

ENetTransport::~ENetTransport() {
    Disconnect();
    ENetRelease();
}

// ---------------------------------------------------------------------------
// DiagnosePortFailure
//   Called only after enet_host_create returns null.  Probes the port with a
//   raw socket to distinguish "address already in use" from other ENet errors.
//   Post-hoc probing avoids two problems with the old upfront-probe approach:
//     1. TOCTOU: port free when probed → taken by the time ENet binds.
//     2. Probe interference: some Windows/ENet version combos fail to bind the
//        same UDP port a second time within the same scheduler quantum even
//        after closesocket(), causing enet_host_create to fail spuriously.
// ---------------------------------------------------------------------------
namespace {

    void DiagnosePortFailure(uint16_t port) {
#if defined(_WIN32)
        SOCKET probe = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (probe == INVALID_SOCKET) {
            std::fprintf(stderr,
                "[ENetTransport] Host: enet_host_create failed on port %u "
                "(diagnostic socket() failed: WSA %d)\n",
                port, WSAGetLastError());
            return;
        }
        sockaddr_in sa = {};
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = INADDR_ANY;
        sa.sin_port = htons(port);
        if (bind(probe, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) == SOCKET_ERROR) {
            int  err = WSAGetLastError();
            char msg[256] = {};
            FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr, static_cast<DWORD>(err), 0, msg, sizeof(msg), nullptr);
            for (int i = static_cast<int>(strlen(msg)) - 1;
                i >= 0 && (msg[i] == '\r' || msg[i] == '\n'); --i)
                msg[i] = '\0';
            std::fprintf(stderr,
                "[ENetTransport] Host: port %u is already in use "
                "(WSA %d: %s)\n", port, err, msg);
        }
        else {
            // Port was actually free — ENet failed for a different reason.
            std::fprintf(stderr,
                "[ENetTransport] Host: enet_host_create failed on port %u "
                "(port appears free; likely an ENet internal error — "
                "check that enet_initialize() succeeded)\n", port);
        }
        closesocket(probe);
#else
        int probe = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (probe < 0) {
            std::fprintf(stderr,
                "[ENetTransport] Host: enet_host_create failed on port %u "
                "(diagnostic socket() failed: errno %d: %s)\n",
                port, errno, strerror(errno));
            return;
        }
        sockaddr_in sa = {};
        sa.sin_family = AF_INET;
        sa.sin_addr.s_addr = INADDR_ANY;
        sa.sin_port = htons(port);
        if (bind(probe, reinterpret_cast<sockaddr*>(&sa), sizeof(sa)) < 0) {
            std::fprintf(stderr,
                "[ENetTransport] Host: port %u is already in use "
                "(errno %d: %s)\n", port, errno, strerror(errno));
        }
        else {
            std::fprintf(stderr,
                "[ENetTransport] Host: enet_host_create failed on port %u "
                "(port appears free; likely an ENet internal error — "
                "check that enet_initialize() succeeded)\n", port);
        }
        close(probe);
#endif
    }

} // namespace

bool ENetTransport::Host(uint16_t port, int maxClients) {
    assert(!impl_->host && "Host called while already active");

    ENetAddress address{};
    address.host = ENET_HOST_ANY;
    address.port = port;

    impl_->host = enet_host_create(&address,
        static_cast<size_t>(maxClients),
        Impl::CHANNEL_COUNT, 0, 0);
    if (!impl_->host) {
        DiagnosePortFailure(port);
        return false;
    }

    impl_->isServer = true;
    impl_->connectionDesc = "0.0.0.0:" + std::to_string(port);
    std::fprintf(stdout, "[ENetTransport] Hosting on port %u (max %d clients)\n",
        port, maxClients);
    return true;
}

bool ENetTransport::Connect(const std::string& address, uint16_t port) {
    assert(!impl_->host && "Connect called while already active");

    impl_->host = enet_host_create(nullptr, 1, Impl::CHANNEL_COUNT, 0, 0);
    if (!impl_->host) {
        std::fprintf(stderr, "[ENetTransport] enet_host_create failed (client)\n");
        return false;
    }

    ENetAddress enetAddr{};
    enet_address_set_host(&enetAddr, address.c_str());
    enetAddr.port = port;

    impl_->serverPeer = enet_host_connect(impl_->host, &enetAddr,
        Impl::CHANNEL_COUNT, 0);
    if (!impl_->serverPeer) {
        std::fprintf(stderr, "[ENetTransport] enet_host_connect failed\n");
        enet_host_destroy(impl_->host);
        impl_->host = nullptr;
        return false;
    }

    impl_->isServer = false;
    impl_->connectionDesc = address + ":" + std::to_string(port);
    std::fprintf(stdout, "[ENetTransport] Connecting to %s:%u\n", address.c_str(), port);
    return true;
}

void ENetTransport::Disconnect() {
    if (!impl_->host) return;

    for (auto& [id, peer] : impl_->peerById)
        enet_peer_disconnect(peer, 0);

    enet_host_flush(impl_->host);
    enet_host_destroy(impl_->host);

    impl_->host = nullptr;
    impl_->serverPeer = nullptr;
    impl_->peerById.clear();
    impl_->idByPeer.clear();
    impl_->nextPeerId = 1;
    impl_->connectionDesc.clear();
}

void ENetTransport::Poll() {
    if (!impl_->host) return;

    ENetEvent event;
    while (enet_host_service(impl_->host, &event, 0) > 0) {
        switch (event.type) {
        case ENET_EVENT_TYPE_CONNECT:
            impl_->HandleConnect(event, onPeerConnected);
            break;
        case ENET_EVENT_TYPE_DISCONNECT:
            impl_->HandleDisconnect(event, onPeerDisconnected);
            break;
        case ENET_EVENT_TYPE_RECEIVE:
            impl_->HandleReceive(event, onPacketReceived);
            break;
        default:
            break;
        }
    }
}

void ENetTransport::Send(uint8_t peerId, const uint8_t* data,
    size_t length, bool reliable) {
    ENetPeer* peer = impl_->PeerForId(peerId);
    if (!peer) {
        std::fprintf(stderr, "[ENetTransport] Send: unknown peerId %u\n", peerId);
        return;
    }
    ENetPacket* pkt = enet_packet_create(data, length, impl_->SendFlags(reliable));
    enet_peer_send(peer, impl_->Channel(reliable), pkt);
}

void ENetTransport::Broadcast(const uint8_t* data, size_t length, bool reliable) {
    if (!impl_->host) return;
    ENetPacket* pkt = enet_packet_create(data, length, impl_->SendFlags(reliable));
    enet_host_broadcast(impl_->host, impl_->Channel(reliable), pkt);
}

void ENetTransport::BroadcastExcept(uint8_t excludePeerId,
    const uint8_t* data, size_t length, bool reliable) {
    if (!impl_->host) return;
    ENetPeer* exclude = impl_->PeerForId(excludePeerId);
    for (auto& [id, peer] : impl_->peerById) {
        if (peer == exclude) continue;
        ENetPacket* pkt = enet_packet_create(data, length, impl_->SendFlags(reliable));
        enet_peer_send(peer, impl_->Channel(reliable), pkt);
    }
}

bool ENetTransport::IsConnected() const {
    if (!impl_->host) return false;
    if (impl_->isServer) return impl_->host->connectedPeers > 0;
    return impl_->serverPeer &&
        impl_->serverPeer->state == ENET_PEER_STATE_CONNECTED;
}

int ENetTransport::GetPeerCount() const {
    if (!impl_->host) return 0;
    return static_cast<int>(impl_->host->connectedPeers);
}

std::string ENetTransport::GetConnectionString() const {
    return impl_->connectionDesc;
}

bool ENetTransport::TryConnectOrHost(const std::string& address, uint16_t port,
    int maxClients, uint32_t timeoutMs) {
    if (!Connect(address, port)) {
        // Connect() itself failed to create the client host — go straight to server
        Host(port, maxClients);
        return false;
    }

    // Poll in short increments so we can bail out cleanly as soon as we know
    // the connection failed, rather than blocking for the full timeout with a
    // single enet_host_service call (which keeps the client socket open until
    // it returns, delaying the server host_create on the same machine).
    constexpr enet_uint32 STEP_MS = 50;
    const enet_uint32 steps = (static_cast<enet_uint32>(timeoutMs) + STEP_MS - 1) / STEP_MS;

    int lastResult = 0;
    for (enet_uint32 i = 0; i < steps; ++i) {
        ENetEvent event;
        lastResult = enet_host_service(impl_->host, &event, STEP_MS);

        if (lastResult > 0) {
            if (event.type == ENET_EVENT_TYPE_CONNECT) {
                impl_->HandleConnect(event, onPeerConnected);
                std::fprintf(stdout, "[ENetTransport] TryConnectOrHost: "
                    "connected as client to %s:%u\n", address.c_str(), port);
                return true;
            }
            if (event.type == ENET_EVENT_TYPE_DISCONNECT) {
                // Server actively refused — no point waiting further
                std::fprintf(stdout, "[ENetTransport] TryConnectOrHost: "
                    "connection to %s:%u actively refused\n", address.c_str(), port);
                break;
            }
        }
        // lastResult == 0: nothing yet, keep waiting
        if (lastResult < 0) break; // service error
    }

    // Connection failed — log the specific reason, then tear down the client
    // host before binding the server socket on the same port.
    std::fprintf(stdout,
        "[ENetTransport] TryConnectOrHost: no server found at %s:%u "
        "(%s), becoming server on port %u\n",
        address.c_str(), port,
        (lastResult < 0 ? "service error" : "connection timed out"),
        port);

    // TeardownClientHost() calls disconnect_now + flush + destroy, which is
    // necessary on Windows where enet_host_destroy may not release the socket
    // immediately.  Centralising the teardown here also removes the duplicate
    // inline sequence that was previously in this function.
    impl_->TeardownClientHost();

    Host(port, maxClients);
    return false;
}

NetworkStat ENetTransport::GetStat() const
{
    NetworkStat stat{};

    if (!impl_->host) {
        return stat;
    }

    // 1. Host-level stats (Total data across all connections)
    stat.incomingBytesTotal = impl_->host->totalReceivedData;
    stat.outgoingBytesTotal = impl_->host->totalSentData;

    // 2. Peer-level stats (RTT and Packet Loss)
    if (!impl_->isServer && impl_->serverPeer) {
        // Client Mode: Pull directly from the server connection

        // ENet scales packet loss by ENET_PEER_PACKET_LOSS_SCALE (65536).
        // Divide by the scale to get a 0.0 - 1.0 ratio, then multiply by 100 for percent.
        stat.packetLossPercent = (static_cast<float>(impl_->serverPeer->packetLoss) / ENET_PEER_PACKET_LOSS_SCALE) * 100.0f;
        stat.packetLossPercentVariance = (static_cast<float>(impl_->serverPeer->packetLossVariance) / ENET_PEER_PACKET_LOSS_SCALE) * 100.0f;

        // Cap RTT to the max of uint16_t to prevent overflow wrapping (ENet RTT is uint32_t)
        stat.roundTripTime = static_cast<uint16_t>(std::min<enet_uint32>(impl_->serverPeer->roundTripTime, UINT16_MAX));
        stat.roundTripTimeVariance = static_cast<uint16_t>(std::min<enet_uint32>(impl_->serverPeer->roundTripTimeVariance, UINT16_MAX));

    }
    else if (impl_->isServer && !impl_->peerById.empty()) {
        // Server Mode: Average the stats across all active clients
        float totalLoss = 0.0f;
        float totalLossVar = 0.0f;
        enet_uint32 totalRtt = 0;
        enet_uint32 totalRttVar = 0;

        int count = 0;
        for (const auto& [id, peer] : impl_->peerById) {
            totalLoss += static_cast<float>(peer->packetLoss);
            totalLossVar += static_cast<float>(peer->packetLossVariance);
            totalRtt += peer->roundTripTime;
            totalRttVar += peer->roundTripTimeVariance;
            count++;
        }

        if (count > 0) {
            stat.packetLossPercent = (totalLoss / count / ENET_PEER_PACKET_LOSS_SCALE) * 100.0f;
            stat.packetLossPercentVariance = (totalLossVar / count / ENET_PEER_PACKET_LOSS_SCALE) * 100.0f;

            enet_uint32 avgRtt = totalRtt / count;
            enet_uint32 avgRttVar = totalRttVar / count;

            stat.roundTripTime = static_cast<uint16_t>(std::min<enet_uint32>(avgRtt, UINT16_MAX));
            stat.roundTripTimeVariance = static_cast<uint16_t>(std::min<enet_uint32>(avgRttVar, UINT16_MAX));
        }
    }

    return stat;
}