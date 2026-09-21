#include "server/ServerApp.h"
#include "server/Player.h"

#include <iostream>
#include <string_view>
#include <raylib.h>

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

    World world {};

    Controller controller;
    Player player(controller, {67.0f, 67.0f});

    world.appendPlayer(player, controller);

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
    std::cout << "Peer connected from " << peer->address.host << ":" << peer->address.port
              << " (" << connectedPeers_ << " total).\n";
}

void ServerApp::onReceive(ENetPeer* peer, ENetPacket* packet, uint8_t channelID) {
    const std::string_view payload(reinterpret_cast<const char*>(packet->data),
                                   packet->dataLength);

    std::cout << "Received " << payload.size()
              << " bytes on channel " << static_cast<int>(channelID)
              << ": " << payload << '\n';

    context_.send(peer, payload, channelID);
}

void ServerApp::onDisconnect(ENetPeer* peer) {
    if (connectedPeers_ > 0) {
        --connectedPeers_;
    }
    std::cout << "Peer disconnected from " << peer->address.host << ":" << peer->address.port
              << " (" << connectedPeers_ << " remaining).\n";
}

void ServerApp::onTick(uint64_t tick) {
    if (WindowShouldClose()) {
        stop();
    }
    world_.update();
}

} // namespace server
