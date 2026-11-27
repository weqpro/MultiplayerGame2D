#ifndef SERVER_SERVER_APP_H
#define SERVER_SERVER_APP_H

#include <cstddef>
#include <cstdint>
#include <unordered_map>

#include <enet/enet.h>

#include "shared/AppBase.h"
#include "shared/Protocol.h"
#include "shared/Renderer.h"
#include "shared/types/Action.h"
#include "shared/types/ActiveAttack.h"
#include "shared/types/Ids.h"
#include "server/Replicator.h"
#include "server/controllers/NetworkController.h"
#include "server/WebServerContext.h"
#include "server/World.h"

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

    void onTick(uint64_t tick);

    void onShutDown();

    void handle(ENetPeer* peer, const shared::ControlsMessage& message);

    template <typename A>
    void handle(ENetPeer* peer, const shared::ActionMessage<A>& message) {
        enqueueAction(peer, message.action);
    }

    void enqueueAction(ENetPeer* peer, const shared::Action& action);

    [[nodiscard]] NetworkController* networkControllerOf(ENetPeer* peer);

    uint32_t serviceTimeout() const { return 1; }

    uint16_t port_;
    size_t peerCount_;
    size_t connectedPeers_{0};

    World world_;
    Replicator replicator_{context_};
    shared::Renderer renderer_;

    std::unordered_map<ENetPeer*, shared::EntityId> sessions_;
    shared::EntityId hostPawn_{shared::NULL_ENTITY};
    std::vector<shared::ActiveAttack> activeAttacks_;
#ifdef ENABLE_DEBUG_DRAW
    bool showFlowField_ = false;
#endif
};

} // namespace server

#endif // SERVER_SERVER_APP_H
