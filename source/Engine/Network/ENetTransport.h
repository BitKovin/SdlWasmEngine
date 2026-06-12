#pragma once

#include "INetworkTransport.h"
#include <cstdint>
#include <memory>
#include <string>

// ---------------------------------------------------------------------------
// ENetTransport
//
// INetworkTransport implementation backed by ENet 1.3.x.
//
// ENet headers are confined to ENetTransport.cpp — this header is clean of
// any Windows SDK or ENet includes, so it is safe to include anywhere.
//
// Channel layout (internal):
//   Channel 0 — reliable ordered     (spawns, despawns, RPCs, handshake)
//   Channel 1 — unreliable unordered (entity state updates)
//
// Peer ID assignment:
//   Server: assigns IDs 1–254 to incoming clients in connection order.
//           Peer ID 0 is always the server itself.
//   Client: the server is always addressed as peerId = 0.
//
// Usage:
//   ENetTransport transport;
//   transport.Host(7777, 4);                 // server
//   transport.Connect("127.0.0.1", 7777);   // client
//   // each frame:
//   transport.Poll();
// ---------------------------------------------------------------------------

class ENetTransport : public INetworkTransport {
public:
    ENetTransport();
    ~ENetTransport() override;

    // Non-copyable, non-movable (owns ENet host state)
    ENetTransport(const ENetTransport&)            = delete;
    ENetTransport& operator=(const ENetTransport&) = delete;

    // INetworkTransport
    bool Host(uint16_t port, int maxClients) override;
    bool Connect(const std::string& address, uint16_t port) override;
    void Disconnect() override;
    void Poll() override;

    void Send(uint8_t peerId, const uint8_t* data, size_t length, bool reliable) override;
    void Broadcast(const uint8_t* data, size_t length, bool reliable) override;
    void BroadcastExcept(uint8_t excludePeerId,
                         const uint8_t* data, size_t length, bool reliable) override;

    bool IsConnected() const override;
    int  GetPeerCount() const override;

    // Convenience for testing: try to connect to address:port, blocking up to
    // timeoutMs for the result.  If the connection succeeds, returns true and
    // this transport operates as a client.  On timeout or failure, tears down
    // the client host, starts hosting on the same port, and returns false.
    //
    //   bool isServer = !transport.TryConnectOrHost("127.0.0.1", 7777, 4);
    //   NetworkManager::Init(&transport, isServer);
    bool TryConnectOrHost(const std::string& address, uint16_t port,
                          int maxClients, uint32_t timeoutMs = 1000);

private:
    // All ENet types (ENetHost*, ENetPeer*, etc.) are hidden inside Impl.
    // Defined in ENetTransport.cpp where enet/enet.h is included.
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
