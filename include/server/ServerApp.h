#ifndef SERVER_APP__H_
#define SERVER_APP__H_

#include <cstddef>
#include <cstdint>

#include <enet/enet.h>

#include "shared/AppBase.h"
#include "server/WebServerContext.h"

namespace server {

class ServerApp : public shared::AppBase<ServerApp, WebServerContext> {
    using Base = shared::AppBase<ServerApp, WebServerContext>;

    friend Base;

public:
    ServerApp(uint16_t port, size_t peerCount);

    void onConnect(ENetPeer* peer);

    void onReceive(ENetPeer* peer, ENetPacket* packet, uint8_t channelID);

    void onDisconnect(ENetPeer* peer);

    size_t connectedPeers() const noexcept { return connectedPeers_; }

private:
    bool onStartUp();

    void onShutDown();

    uint32_t serviceTimeout() const { return 1; }

    uint16_t port_;
    size_t peerCount_;
    size_t connectedPeers_{0};
};

} // namespace server

#endif // SERVER_APP__H_
