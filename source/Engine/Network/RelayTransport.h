#pragma once

#include "INetworkTransport.h"
#include <cstdint>
#include <memory>
#include <string>

// ---------------------------------------------------------------------------
// RelayTransport
//
// INetworkTransport implementation that never connects peers directly.
// Every client connects to one known relay server (see RelayServer/, the
// ASP.NET Core project that ships alongside this code) which creates/joins
// "rooms" by UUID room code and relays traffic between the peers in a room.
//
// Channel layout:
//   Reliable channel   -> TCP to the relay (native build) / WebSocket (Emscripten)
//   Unreliable channel -> UDP to the relay (native build) / WebSocket (Emscripten,
//                          downgraded to reliable — browsers have no raw UDP).
//                          Native build: if the UDP path isn't registered
//                          yet (brief window right after connecting), an
//                          unreliable Send is simply dropped rather than
//                          silently promoted to reliable.
//
// Peer ID assignment (done by the relay, not locally):
//   Room creator (CreateRoom)  -> peerId 0  (the room's "host")
//   Each JoinRoom() caller     -> next free id, 1..maxClients-1
//
// Unlike ENetTransport, the relay process is never itself a peer — there is
// no "isServer" concept here. The room's host is just whichever connected
// peer currently has the lowest peerId; see GetHostPeerId(). What happens
// when the host disconnects is controlled per-room by the
// closeRoomOnHostLeave flag passed to CreateRoom() (see below) and
// INetworkTransport::onHostChanged.
//
// Two .cpp implementations exist; exactly one is compiled per build:
//   RelayTransport_Native.cpp     — !defined(__EMSCRIPTEN__). TCP+UDP via
//                                    third_party/kissnet.hpp.
//   RelayTransport_Emscripten.cpp — defined(__EMSCRIPTEN__). WebSocket via
//                                    emscripten/websocket.h.
// Both privately include RelayProtocol.h for the wire format — never
// include that header from engine/game code.
//
// Usage:
//   RelayTransport host("relay.example.com", 7777, 7778);
//   host.CreateRoom(8);                                // -> onPeerConnected(0)
//   std::string code = host.GetConnectionString();     // share this with friends
//
//   // ...on another machine:
//   RelayTransport joiner("relay.example.com", 7777, 7778);
//   joiner.JoinRoom(code);                              // -> onPeerConnected(assignedId)
//
//   // each frame, on every instance:
//   transport.Poll();
// ---------------------------------------------------------------------------

class RelayTransport : public INetworkTransport {
public:
    // relayHost/relayTcpPort: control + reliable channel. On native builds
    //   this is a raw TCP endpoint; on Emscripten it's the host/port used to
    //   build the ws:// (or wss://, see RelayTransport_Emscripten.cpp) URL.
    // relayUdpPort: unreliable channel for native builds only. Ignored on
    //   Emscripten — a browser has no raw UDP socket access.
    RelayTransport(std::string relayHost, uint16_t relayTcpPort, uint16_t relayUdpPort);
    ~RelayTransport() override;

    RelayTransport(const RelayTransport&)            = delete;
    RelayTransport& operator=(const RelayTransport&) = delete;

    // --- INetworkTransport ---------------------------------------------
    // Host(): kept only so code written against ENetTransport still
    //   compiles unmodified. `port` is ignored — the relay endpoint is
    //   fixed at construction time, not per-call. Equivalent to
    //   CreateRoom(maxClients, /*closeRoomOnHostLeave=*/false).
    bool Host(uint16_t port, int maxClients) override;

    // Connect(): `address` is interpreted as the ROOM CODE (UUID) to join.
    //   `port` is ignored. Equivalent to JoinRoom(address).
    bool Connect(const std::string& address, uint16_t port) override;

    void Disconnect() override;
    void Poll() override;

    void Send(uint8_t peerId, const uint8_t* data, size_t length, bool reliable) override;
    void Broadcast(const uint8_t* data, size_t length, bool reliable) override;
    void BroadcastExcept(uint8_t excludePeerId,
                         const uint8_t* data, size_t length, bool reliable) override;

    bool IsConnected() const override;
    int  GetPeerCount() const override;
    NetworkStat GetStat() const override;

    // Room code (UUID) of the current session, or empty if not connected.
    std::string GetConnectionString() const override;

    // --- Relay-specific API: prefer these over Host()/Connect() in new code
    //
    // closeRoomOnHostLeave:
    //   false (default) — if peerId 0 disconnects, the relay migrates the
    //     room's host to the next-lowest connected peerId and notifies
    //     every remaining peer via onHostChanged. The room keeps running.
    //   true — if peerId 0 disconnects, the relay closes the room and
    //     disconnects every remaining peer (onPeerDisconnected fires for
    //     each of them). onHostChanged never fires for this room.
    bool CreateRoom(int maxClients, bool closeRoomOnHostLeave = false);
    bool JoinRoom(const std::string& roomCode);
    void LeaveRoom();

    uint8_t GetLocalPeerId() const; // 0xFF if not currently in a room
    uint8_t GetHostPeerId()  const; // 0xFF if not currently in a room

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
