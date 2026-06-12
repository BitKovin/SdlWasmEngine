#pragma once

#include <cstdint>
#include <functional>
#include <string>

// ---------------------------------------------------------------------------
// INetworkTransport
//
// Abstract transport layer. NetworkManager talks only to this interface.
// Concrete implementations: ENetTransport, SteamTransport, EOSTransport, etc.
//
// Threading model: all callbacks fire on the thread that calls Poll().
// Poll() must be called once per frame from the same thread as the engine
// main loop. No locking is performed by NetworkManager itself.
// ---------------------------------------------------------------------------

class INetworkTransport {
public:
    virtual ~INetworkTransport() = default;

    // -----------------------------------------------------------------------
    // Callbacks — set by NetworkManager before calling any other method.
    // All three must be set before Connect/Host is called.
    // -----------------------------------------------------------------------

    // Fired when a peer connects. peerId is assigned by the transport layer
    // for server-hosted connections (0–254). On the client side this fires
    // once for the server connection and peerId is always 0.
    std::function<void(uint8_t peerId)> onPeerConnected;

    // Fired when a peer disconnects.
    std::function<void(uint8_t peerId)> onPeerDisconnected;

    // Fired when a packet arrives from a peer.
    std::function<void(uint8_t senderId, const uint8_t* data, size_t length)> onPacketReceived;

    // -----------------------------------------------------------------------
    // Lifecycle
    // -----------------------------------------------------------------------

    // Start listening for incoming connections on the given port.
    // maxClients: maximum simultaneous connected peers.
    // Returns true on success.
    virtual bool Host(uint16_t port, int maxClients) = 0;

    // Connect to a remote host (client use).
    // Returns true if the connection attempt was initiated (not yet established).
    // onPeerConnected fires once the handshake completes.
    virtual bool Connect(const std::string& address, uint16_t port) = 0;

    // Disconnect all peers and shut down the transport.
    // Safe to call even if not connected.
    virtual void Disconnect() = 0;

    // -----------------------------------------------------------------------
    // Per-frame
    // -----------------------------------------------------------------------

    // Service the network layer: dispatch incoming events and fire callbacks.
    // Must be called once per frame on the main thread.
    virtual void Poll() = 0;

    // -----------------------------------------------------------------------
    // Sending
    // -----------------------------------------------------------------------

    // Send raw bytes to a specific peer.
    // reliable=true: guaranteed delivery, ordered. false: unreliable, unordered.
    // Call only when connected.
    virtual void Send(uint8_t peerId, const uint8_t* data, size_t length, bool reliable) = 0;

    // Send to all connected peers.
    virtual void Broadcast(const uint8_t* data, size_t length, bool reliable) = 0;

    // Send to all connected peers except one (server relay helper).
    virtual void BroadcastExcept(uint8_t excludePeerId,
                                  const uint8_t* data, size_t length, bool reliable) = 0;

    // -----------------------------------------------------------------------
    // State queries
    // -----------------------------------------------------------------------

    virtual bool IsConnected() const = 0;

    // Returns the number of currently connected peers.
    virtual int GetPeerCount() const = 0;
};
