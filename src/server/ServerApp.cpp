#include "server/ServerApp.h"

#include <cstddef>
#include <iostream>
#include <iterator>
#include <span>
#include <string>
#include <raylib.h>

#include "server/controllers/NetworkController.h"
#include "server/controllers/WanderController.h"
#include "shared/Messages.h"
#include "shared/Protocol.h"
#include "shared/controllers/KeyboardController.h"
#include "shared/entities/NPC.h"
#include "shared/entities/Player.h"

namespace server {
namespace {

constexpr uint32_t SHUTDOWN_DRAN_STEPS_MS = 50;

constexpr int SHUTDOWN_DRAIN_STEPS = 20;

constexpr Color PLAYER_COLORS[] = {
    DARKGRAY, MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN
};

constexpr int NPC_COUNT = 3;

constexpr float NPC_LEASH = 120.0f;

Color randomPlayerColor() {
    const int last = static_cast<int>(std::size(PLAYER_COLORS)) - 1;
    return PLAYER_COLORS[GetRandomValue(0, last)];
}

Vector2 randomPoint(int minX, int maxX, int minY, int maxY) {
    return {static_cast<float>(GetRandomValue(minX, maxX)), static_cast<float>(GetRandomValue(minY, maxY))};
}

} // namespace

ServerApp::ServerApp(uint16_t port, size_t peerCount)
    : Base(port, peerCount),
      port_(port),
      peerCount_(peerCount) {}

bool ServerApp::onStartUp() {
    std::cout << "Server listening on port " << port_
              << " (up to " << peerCount_ << " peers).\n";

    auto& host = world_.spawn<shared::Player>(shared::Body{.pos = {67.0f, 67.0f}},
                                              shared::Appearance{.color = randomPlayerColor()},
                                              "host");
    world_.setMap();
    renderer_.loadAtlas();

    constexpr float TILE = 16.0f;
    auto& [cam, camCtrl] = world_.camera();
    const auto& map = world_.worldMap();
    camCtrl.bounds = {0, 0, static_cast<float>(map.width()) * TILE, static_cast<float>(map.height()) * TILE};
    camCtrl.snapTo(cam, host.body().pos);

    world_.possess<shared::KeyboardController>(host);
    hostPawn_ = host.id();

    for (int i = 0; i < NPC_COUNT; ++i) {
        const Vector2 home = randomPoint(250, 550, 120, 330);
        auto& npc = world_.spawn<shared::NPC>(shared::Body{.pos = home},
                                              shared::Appearance{.color = GRAY, .radius = 7.0f});
        world_.possess<WanderController>(npc, home, NPC_LEASH);
    }
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

    for (int i = 0; i < SHUTDOWN_DRAIN_STEPS && connectedPeers_ > 0; ++i) {
        service(SHUTDOWN_DRAN_STEPS_MS);
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

    auto& pawn = world_.spawn<shared::Player>(shared::Body{.pos = randomPoint(100, 200, 100, 200)},
                                              shared::Appearance{.color = randomPlayerColor()},
                                              "n n " + std::to_string(peer->incomingPeerID + 1));
    world_.possess<NetworkController>(pawn);
    sessions_[peer] = pawn.id();
    replicator_.join(peer, pawn.id());

    std::cout << "Peer connected from " << peer->address.host << ":" << peer->address.port
              << " as entity " << pawn.id() << " (" << connectedPeers_ << " total).\n";
}

void ServerApp::onReceive(ENetPeer* peer, ENetPacket* packet, uint8_t /*channelID*/) {
    const std::span<const std::byte> data(reinterpret_cast<const std::byte*>(packet->data), packet->dataLength);
    const auto type = shared::peek_type(data);
    if (!type) {
        //TODO: handle
        std::cout << "Invalid type" << std::endl;
        return;
    }
    switch (*type) {
        case shared::MessageType::INTENT: {
            const auto received = shared::Packet<shared::IntentMessage>::deserialize(data);
            if (!received) {
                //TODO: handle
                std::cout << "Deserialize failed" << std::endl;
                break;
            }
            const auto session = sessions_.find(peer);
            if (session == sessions_.end()) {
                break;
            }
            if (auto* controller = world_.controllerOf<NetworkController>(session->second)) {
                controller->submit(received->message.seq, received->message.intent);
            }
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

    replicator_.leave(peer);
    const auto session = sessions_.find(peer);
    if (session == sessions_.end()) {
        return;
    }
    world_.despawn(session->second);
    sessions_.erase(session);
}

void ServerApp::onTick(uint64_t tick) {
    if (WindowShouldClose()) {
        stop();
    }
    world_.update(tickSeconds());
    replicator_.flush(world_, tick);

    auto& [cam, camCtrl] = world_.camera();
    auto me = world_.entities().find<shared::Player>(hostPawn_);
    camCtrl.update(cam, tickSeconds(), me->body().pos, me->body().pos - me->prevPos);
    renderer_.render(world_.entities(), world_.worldMap(), cam, hostPawn_);
}

} // namespace server
