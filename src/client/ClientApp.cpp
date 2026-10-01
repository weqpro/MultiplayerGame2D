#include "client/ClientApp.h"
#include "client/World.h"

#include <cstddef>
#include <enet/enet.h>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <utility>

#include "shared/Messages.h"

namespace client {
namespace {

constexpr uint32_t kShutdownDrainStepMs = 50;

constexpr int kShutdownDrainSteps = 20;

} // namespace

ClientApp::ClientApp(uint16_t port, size_t peerCount)
    : ClientApp("127.0.0.1", port, peerCount) {
    world_ = std::make_unique<World>();
    renderer_ = std::make_unique<Renderer>();
}

ClientApp::ClientApp(std::string hostName, uint16_t port, size_t peerCount)
    : Base(peerCount),
      hostName_(std::move(hostName)),
      port_(port) {
    world_ = std::make_unique<World>();
    renderer_ = std::make_unique<Renderer>();
}

bool ClientApp::onStartUp() {
    std::cout << "Initiating connection to " << hostName_ << ":" << port_ << "...\n";

    peer_ = context_.connect(hostName_, port_);
    if (!peer_) {
        std::cerr << "ClientApp startup failed: Could not initiate connection.\n";
        return false;
    }

    return true;
}

void ClientApp::onTick(uint64_t tick) {
    if (WindowShouldClose()) {
        stop();
    }
    if (!peer_ || peer_->state != ENET_PEER_STATE_CONNECTED) {
        return;
    }

    world_->update(1.0f/TPS);
    context_.send(peer_, world_->messageBuf, 0,0);
    renderer_->render();
}

void ClientApp::onShutDown() {
    if (peer_ && peer_->state == ENET_PEER_STATE_CONNECTED) {
        enet_peer_disconnect(peer_, 0);

        for (int i = 0; i < kShutdownDrainSteps && peer_; ++i) {
            service(kShutdownDrainStepMs);
        }

        if (peer_) {
            enet_peer_reset(peer_);
        }
    }
    peer_ = nullptr;
}

void ClientApp::onConnect(ENetPeer* /*peer*/) {
    std::cout << "Connected to server.\n";
}

void ClientApp::onReceive(ENetPeer* peer, ENetPacket* packet, uint8_t channelID) {
    std::span<std::byte> data(reinterpret_cast<std::byte*>(packet->data), packet->dataLength); auto mType = shared::peek_type(data);
    if (!mType) {
        //TODO: handle 
        std::cout << "Invalid type" << std::endl;
        return;
    }
    switch (mType.value()) {
        case shared::MessageType::PLAYERS: {
            auto packet = shared::Packet<shared::PlayersMessage>::deserialize(data);
            if (!packet) {
                //TODO: handle
                std::cout << "Deserialize failed" << std::endl;
                break;
            }
            auto players = packet->message;
            world_->players_ = {};
            world_->players_.reserve(players.players.size());
            renderer_->bodies_ = {};
            renderer_->bodies_.reserve(players.players.size());
            for (auto& player : players.players) {
                world_->players_[player.id]->body.pos = player.body.pos;
                renderer_->addBody(&world_->players_[player.id]->body);
            }
            break;
        }
        default:
            break;
    }
}

void ClientApp::onDisconnect(ENetPeer* /*peer*/) {
    std::cout << "Server disconnected. Terminating game loop.\n";
    peer_ = nullptr;
    stop();
}

} // namespace client
