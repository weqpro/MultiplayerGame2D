#ifndef CLIENT_APP__H_
#define CLIENT_APP__H_

#include <cstdint>
#include <string>

#include <enet/enet.h>

#include "shared/AppBase.h"
#include "client/WebClientContext.h"

namespace client {

/**
 * @brief Client application: connects to a single server and drives a
 *        fixed-timestep loop with a variable-rate frame on top.
 *
 * Built on shared::AppBase, which owns the WebClientContext, the loop and the
 * timing. This class supplies the lifecycle hooks and the ENet handler
 * callbacks.
 *
 * @code
 * client::ClientApp app(1234);
 * shared::LoopConfig cfg;
 * cfg.fixedHz = 60;   // simulation steps per second
 * cfg.frameHz = 120;  // headless: loop paces itself
 * app.run(cfg);
 * @endcode
 */
class ClientApp : public shared::AppBase<ClientApp, WebClientContext> {
    using Base = shared::AppBase<ClientApp, WebClientContext>;

    /** @brief Grants the base static access to the private lifecycle hooks. */
    friend Base;

public:
    /**
     * @brief Constructs a client targeting localhost.
     *
     * @param port      Server port to connect to.
     * @param peerCount Number of outgoing connections the context allocates.
     */
    ClientApp(uint16_t port, size_t peerCount = 1);

    /**
     * @brief Constructs a client targeting an explicit host.
     *
     * @param hostName  Server host name or address.
     * @param port      Server port to connect to.
     * @param peerCount Number of outgoing connections the context allocates.
     *
     * @note The base subobject, and with it the context, is constructed before
     *       @c hostName_ and @c port_; only @p peerCount reaches the context.
     */
    ClientApp(std::string hostName, uint16_t port, size_t peerCount = 1);

    /**
     * @brief Called once the connection to the server is established.
     * @param peer The connected server peer.
     */
    void onConnect(ENetPeer* peer);

    /**
     * @brief Called for each packet received from the server.
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
     * @brief Called when the server connection is lost or closed.
     *
     * Clears the peer and stops the loop.
     *
     * @param peer The disconnected peer.
     */
    void onDisconnect(ENetPeer* peer);

private:
    /**
     * @brief Initiates the connection to the server.
     * @return @c true if the attempt was started, @c false to abort the run.
     */
    bool onStartUp();

    /**
     * @brief Sends one payload per fixed step once the connection is up.
     * @param tick Zero-based index of the current fixed step.
     */
    void onFixedUpdate(uint64_t tick);

    /**
     * @brief Prints loop statistics once per second.
     *
     * Headless stand-in for an on-screen HUD: frames/s, fixed steps/s, the
     * current interpolation factor and total clamped-away time.
     *
     * @param frameDelta Real time consumed by this frame, after clamping.
     */
    void onUpdate(Duration frameDelta);

    /**
     * @brief Closes the connection gracefully, draining the handshake.
     *
     * Falls back to a local reset if the server does not acknowledge in time.
     */
    void onShutDown();

    std::string hostName_;      ///< Server host name or address.
    uint16_t port_;             ///< Server port.
    ENetPeer* peer_{nullptr};   ///< Server peer; null when not connected.

    Duration statsElapsed_{Duration::zero()}; ///< Time since the last stats line.
    uint64_t statsFrame_{0};                  ///< frameCount() at the last stats line.
    uint64_t statsTick_{0};                   ///< tickCount() at the last stats line.
};

} // namespace client

#endif // CLIENT_APP__H_
