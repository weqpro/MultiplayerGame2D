#include "client/ClientApp.h"
#include "client/World.h"

#include <cstddef>
#include <iostream>
#include <memory>
#include <span>
#include <string>
#include <utility>

namespace client {
namespace {

constexpr uint32_t kShutdownDrainStepMs = 50;

constexpr int kShutdownDrainSteps = 20;

} // namespace

ClientApp::ClientApp(uint16_t port, size_t peerCount)
    : ClientApp("127.0.0.1", port, peerCount) {
    world_ = std::make_unique<World>(context_);
}

ClientApp::ClientApp(std::string hostName, uint16_t port, size_t peerCount)
    : Base(peerCount),
      hostName_(std::move(hostName)),
      port_(port) {}

bool ClientApp::onStartUp() {
    std::cout << "Initiating connection to " << hostName_ << ":" << port_ << "...\n";

    peer_ = context_.connect(hostName_, port_);
    if (!peer_) {
        std::cerr << "ClientApp startup failed: Could not initiate connection.\n";
        return false;
    }

    world_->joinPlayer({148.8f, 148.8f});
    return true;
}

void ClientApp::onTick(uint64_t tick) {
    if (!peer_ || peer_->state != ENET_PEER_STATE_CONNECTED) {
        return;
    }

    world_->update(0.1f);

    const std::string payload = "Tick: " + std::to_string(tick);
    context_.send(peer_, std::as_bytes(std::span{payload}), 0, ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
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

void ClientApp::onReceive(ENetPeer* /*peer*/, ENetPacket* packet, uint8_t channelID) {
    std::cout << "Received " << packet->dataLength
              << " bytes on channel " << static_cast<int>(channelID) << ".\n";
}

void ClientApp::onDisconnect(ENetPeer* /*peer*/) {
    std::cout << "Server disconnected. Terminating game loop.\n";
    peer_ = nullptr;
    stop();
}

} // namespace client
