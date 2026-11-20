#ifndef CLIENT_CLIENT_APP_H
#define CLIENT_CLIENT_APP_H

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>

#include <enet/enet.h>

#include "client/World.h"
#include "shared/AppBase.h"
#include "client/WebClientContext.h"
#include "shared/Messages.h"
#include "shared/Renderer.h"

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

    template <shared::Message M>
    void dispatch(std::span<const std::byte> data);

    std::string hostName_;
    uint16_t port_;
    ENetPeer* peer_{nullptr};

    World world_;
    shared::Renderer renderer_;
    bool mapReady_ = false;
    bool boundReady_ = false;
};

} // namespace client

#endif // CLIENT_CLIENT_APP_H
