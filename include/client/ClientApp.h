#ifndef CLIENT_APP__H_
#define CLIENT_APP__H_

#include <cstdint>
#include <memory>
#include <string>

#include <enet/enet.h>

#include "client/World.h"
#include "shared/AppBase.h"
#include "client/WebClientContext.h"
#include "client/Renderer.h"

namespace client {

class ClientApp : public shared::AppBase<ClientApp, WebClientContext> {
    using Base = shared::AppBase<ClientApp, WebClientContext>;

    friend Base;

public:
    ClientApp(uint16_t port, size_t peerCount = 1);

    ClientApp(std::string hostName, uint16_t port, size_t peerCount = 1);

    void onConnect(ENetPeer* peer);

    void onReceive(ENetPeer* peer, ENetPacket* packet, uint8_t channelID);

    void onDisconnect(ENetPeer* peer);

private:
    bool onStartUp();

    void onTick(uint64_t tick);

    void onShutDown();

    std::string hostName_;
    uint16_t port_;
    ENetPeer* peer_{nullptr};

    std::unique_ptr<World> world_;
    std::unique_ptr<Renderer> renderer_;
};

} // namespace client

#endif // CLIENT_APP__H_
