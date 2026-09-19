#include "client/ClientApp.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <string>

namespace client {

ClientApp::ClientApp(uint16_t port, size_t peerCount)
    : ClientApp("127.0.0.1", port, peerCount) {}

ClientApp::ClientApp(std::string hostName, uint16_t port, size_t peerCount)
    : hostName_(std::move(hostName)), 
      port_(port), 
      client_(peerCount) {}

void ClientApp::StartUp() {
    std::cout << "Initiating connection to " << hostName_ << ":" << port_ << "...\n";
    
    peer_ = client_.connect(hostName_, port_);
    if (!peer_) {
        std::cerr << "ClientApp startup failed: Could not initiate connection.\n";
    }
}

void ClientApp::GameLoop(uint32_t targetTPS) {
    StartUp();

    if (!peer_) {
        return;
    }

    using namespace std::chrono_literals;
    const auto tickInterval = std::chrono::duration_cast<std::chrono::nanoseconds>(1s) / targetTPS;

    uint64_t i = 0;
    auto nextTick = std::chrono::steady_clock::now();

    while (true) {
        client_.handleEvents(0);

        if (peer_->state == ENET_PEER_STATE_DISCONNECTED) {
            std::cout << "Server disconnected. Terminating game loop.\n";
            break;
        }

        if (peer_->state == ENET_PEER_STATE_CONNECTED) {
            std::string payload = "Tick: " + std::to_string(i);
            client_.send(peer_, payload, 0, ENET_PACKET_FLAG_UNRELIABLE_FRAGMENT);
            ++i;
        }

        nextTick += tickInterval;
        std::this_thread::sleep_until(nextTick);
    }
}

} // namespace client
