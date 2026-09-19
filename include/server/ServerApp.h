#ifndef SERVER_APP__H_
#define SERVER_APP__H_

#include <cstddef>
#include <cstdint>

#include <enet/enet.h>

#include "shared/AppBase.h"
#include "server/WebServerContext.h"

namespace server {

/**
 * @brief Server application: accepts peers and services their traffic on a
 *        fixed-rate loop.
 *
 * Built on shared::AppBase, which owns the WebServerContext, the loop and the
 * tick timing. This class supplies the lifecycle hooks and the ENet handler
 * callbacks.
 *
 * @code
 * server::ServerApp app(1234, 32);
 * app.run(20);
 * @endcode
 */
class ServerApp : public shared::AppBase<ServerApp, WebServerContext> {
    using Base = shared::AppBase<ServerApp, WebServerContext>;

    /** @brief Grants the base static access to the private hooks. */
    friend Base;

public:
    /**
     * @brief Constructs and binds the server.
     *
     * @param port      Port to listen on.
     * @param peerCount Maximum number of simultaneous peers.
     */
    ServerApp(uint16_t port, size_t peerCount);

    /**
     * @brief Called when a peer connects.
     * @param peer The peer that connected.
     */
    void onConnect(ENetPeer* peer);

    /**
     * @brief Called for each packet received from a peer.
     *
     * @param peer      The sending peer.
     * @param packet    The received packet.
     * @param channelID The channel the packet arrived on.
     *
     * @warning The packet is destroyed as soon as this call returns; copy
     *          anything that must outlive it.
     */
    void onReceive(ENetPeer* peer, ENetPacket* packet, uint8_t channelID);

    /**
     * @brief Called when a peer disconnects or times out.
     * @param peer The peer that disconnected.
     */
    void onDisconnect(ENetPeer* peer);

    /** @brief Returns the number of currently connected peers. */
    size_t connectedPeers() const noexcept { return connectedPeers_; }

private:
    /**
     * @brief Reports that the server is listening.
     * @return @c true; binding already happened in the constructor.
     */
    bool onStartUp();

    /** @brief Disconnects every remaining peer and drains the handshakes. */
    void onShutDown();

    /**
     * @brief Milliseconds each poll may block inside @c enet_host_service().
     *
     * A small non-zero value keeps an idle server from spinning on the CPU
     * without meaningfully delaying the tick.
     */
    uint32_t serviceTimeout() const { return 1; }

    uint16_t port_;                 ///< Port the server listens on.
    size_t peerCount_;              ///< Maximum number of simultaneous peers.
    size_t connectedPeers_{0};      ///< Number of currently connected peers.
};

} // namespace server

#endif // SERVER_APP__H_
