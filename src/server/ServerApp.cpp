#include "server/ServerApp.h"
#include "server/Player.h"

#include <cstddef>
#include <iostream>
#include <span>
#include <memory>
#include <string_view>
#include <raylib.h>

#include "shared/Messages.h"
#include "shared/Serialization.h"

namespace server {
namespace {

constexpr uint32_t kShutdownDrainStepMs = 50;

constexpr int kShutdownDrainSteps = 20;

} // namespace

ServerApp::ServerApp(uint16_t port, size_t peerCount)
    : Base(port, peerCount),
      port_(port),
      peerCount_(peerCount) {}

bool ServerApp::onStartUp() {
    std::cout << "Server listening on port " << port_
              << " (up to " << peerCount_ << " peers).\n";


    auto ctr = std::make_unique<Controller>();
    Player *player = new Player{std::move(ctr), {67.0f, 67.0f}};

    world_.joinPlayer(player);
    renderer_.addBody(&player->body);

    return true;
}

void ServerApp::onShutDown() {
    ENetHost* host = context_.getHost();
    if (!host) {
        return;
    }

    std::cout << "Shutting down, disconnecting " << connectedPeers_ << " peer(s).\n";

    for (ENetPeer* peer = host->peers; peer < host->peers + host->peerCount; ++peer) {
        if (peer->state == ENET_PEER_STATE_CONNECTED) {
            enet_peer_disconnect(peer, 0);
        }
    }

    for (int i = 0; i < kShutdownDrainSteps && connectedPeers_ > 0; ++i) {
        service(kShutdownDrainStepMs);
    }

    for (ENetPeer* peer = host->peers; peer < host->peers + host->peerCount; ++peer) {
        if (peer->state != ENET_PEER_STATE_DISCONNECTED) {
            enet_peer_reset(peer);
        }
    }
    connectedPeers_ = 0;
}

void ServerApp::onConnect(ENetPeer* peer) {
    ++connectedPeers_;
    Vector2 spawnPoint {
        static_cast<float>(GetRandomValue(100, 200)),
        static_cast<float>(GetRandomValue(100, 200))
    };
    auto ctrl = std::make_unique<ClientController>();
    Player *player = new Player{std::move(ctrl), spawnPoint};
    size_t *id = new size_t{world_.joinPlayer(player)}; //NOTE: do smth about it
    peer->data = id;
    std::cout << "Peer connected from " << peer->address.host << ":" << peer->address.port
              << " (" << connectedPeers_ << " total).\n";
}

void ServerApp::onReceive(ENetPeer* peer, ENetPacket* packet, uint8_t channelID) {
    std::span<std::byte> data(reinterpret_cast<std::byte*>(packet->data), packet->dataLength);
    auto mType = shared::peek_type(data);
    if (!mType) {
        //TODO: handle 
        std::cout << "Invalid type" << std::endl;
        return;
    }
    switch (mType.value()) {
        case shared::MessageType::INTENT: {
            auto packet = shared::Packet<shared::IntentMessage>::deserialize(data);
            if (!packet) {
                //TODO: handle
                std::cout << "Deserialize failed" << std::endl;
                break;
            }
            auto m = packet->message.intent;
            size_t playerId = *static_cast<size_t*>(peer->data);
            world_.setClientPlayerIntent(playerId, {m});
            break;
        }
        default:
            break;
    }
}

void ServerApp::onDisconnect(ENetPeer* peer) {
    if (connectedPeers_ > 0) {
        --connectedPeers_;
    }
    std::cout << "Peer disconnected from " << peer->address.host << ":" << peer->address.port
              << " (" << connectedPeers_ << " remaining).\n";

    size_t playerId = *static_cast<size_t *>(peer->data);

    world_.removePlayer(playerId);
    delete static_cast<size_t *>(peer->data);
}

void ServerApp::onTick(uint64_t tick) {
    if (WindowShouldClose()) {
        stop();
    }
    world_.update(0.1f);
    renderer_.render();

}

} // namespace server
