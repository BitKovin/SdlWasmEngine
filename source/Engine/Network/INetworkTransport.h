#pragma once

#include <cstdint>
#include <functional>
#include <string>

#include "NetworkStat.h"

// ---------------------------------------------------------------------------
// INetworkTransport
//
// Abstract transport layer. NetworkManager talks only to this interface.
// Concrete implementations: ENetTransport, RelayTransport, SteamTransport,
// EOSTransport, etc.
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
    // onPeerConnected / onPeerDisconnected / onPacketReceived must be set
    // before Connect/Host is called. onHostChanged is optional.
    // -----------------------------------------------------------------------

    // Fired when a peer connects. peerId is assigned by the transport layer
    // for server-hosted connections (0–254). On the client side this fires
    // once for the server connection and peerId is always 0.
    std::function<void(uint8_t peerId)> onPeerConnected;

    // Fired when a peer disconnects.
    std::function<void(uint8_t peerId)> onPeerDisconnected;

    // Fired when a packet arrives from a peer.
    std::function<void(uint8_t senderId, const uint8_t* data, size_t length)> onPacketReceived;

    // Fired when the authoritative "host" of the current session changes —
    // e.g. the peer that created the room/session disconnects and another
    // peer takes over. Optional: most transports never call this.
    // ENetTransport never fires it (its server process is never one of the
    // migratable peers). RelayTransport fires it when host migration is
    // enabled for the room and the previous host (peerId 0) disconnects.
    // NetworkManager does not currently subscribe to this — wiring up
    // migration handling on the game-logic side is left for later — but the
    // hook lives on the interface now so that work doesn't require another
    // interface change.
    std::function<void(uint8_t newHostPeerId)> onHostChanged;

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

    virtual NetworkStat GetStat() const = 0;

    // Returns a transport-specific string another peer can use to join this
    // session. For direct transports (ENetTransport) this is "address:port".
    // For relay-based transports (RelayTransport) this is the room code (a
    // server-issued UUID — see RelayTransport::CreateRoom/JoinRoom). Empty
    // string if not currently hosting or joined.
    virtual std::string GetConnectionString() const = 0;
};
