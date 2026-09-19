#include "client/ClientApp.h"

#include <iostream>
#include <string>
#include <utility>

namespace client {
namespace {

/** @brief Milliseconds each drain iteration blocks during shut-down. */
constexpr uint32_t kShutdownDrainStepMs = 50;

/** @brief Maximum drain iterations to wait for the disconnect acknowledgement. */
constexpr int kShutdownDrainSteps = 20;

} // namespace

ClientApp::ClientApp(uint16_t port, size_t peerCount)
    : ClientApp("127.0.0.1", port, peerCount) {}

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
    return true;
}

void ClientApp::onTick(uint64_t tick) {
    if (!peer_ || peer_->state != ENET_PEER_STATE_CONNECTED) {
        return; // Still handshaking; onDisconnect() handles the failure case.
    }

    const std::string payload = "Tick: " + std::to_string(tick);
    context_.send(peer_, payload, 0, ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
}

void ClientApp::onShutDown() {
    if (peer_ && peer_->state == ENET_PEER_STATE_CONNECTED) {
        enet_peer_disconnect(peer_, 0);

        // onDisconnect() clears peer_ once the acknowledgement arrives.
        for (int i = 0; i < kShutdownDrainSteps && peer_; ++i) {
            service(kShutdownDrainStepMs);
        }

        if (peer_) {
            enet_peer_reset(peer_); // No acknowledgement in time: tear down locally.
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
