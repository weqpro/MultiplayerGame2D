#include "server/ServerApp.h"

#include <iostream>
#include <string_view>

namespace server {
namespace {

/** @brief Milliseconds each drain iteration blocks during shut-down. */
constexpr uint32_t kShutdownDrainStepMs = 50;

/** @brief Maximum drain iterations to wait for outstanding disconnects. */
constexpr int kShutdownDrainSteps = 20;

} // namespace

ServerApp::ServerApp(uint16_t port, size_t peerCount)
    : Base(port, peerCount),
      port_(port),
      peerCount_(peerCount) {}

bool ServerApp::onStartUp() {
    std::cout << "Server listening on port " << port_
              << " (up to " << peerCount_ << " peers).\n";
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

    // onDisconnect() decrements connectedPeers_ as the acknowledgements arrive.
    for (int i = 0; i < kShutdownDrainSteps && connectedPeers_ > 0; ++i) {
        service(kShutdownDrainStepMs);
    }

    for (ENetPeer* peer = host->peers; peer < host->peers + host->peerCount; ++peer) {
        if (peer->state != ENET_PEER_STATE_DISCONNECTED) {
            enet_peer_reset(peer); // No acknowledgement in time: tear down locally.
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

    // Echo the payload back on the same channel.
    context_.send(peer, payload, channelID);
}

void ServerApp::onDisconnect(ENetPeer* peer) {
    if (connectedPeers_ > 0) {
        --connectedPeers_;
    }
    std::cout << "Peer disconnected from " << peer->address.host << ":" << peer->address.port
              << " (" << connectedPeers_ << " remaining).\n";
}

} // namespace server
