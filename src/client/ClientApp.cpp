#include "client/ClientApp.h"

#include <cstddef>
#include <enet/enet.h>
#include <iostream>
#include <span>
#include <string>
#include <type_traits>
#include <utility>

#include "shared/Messages.h"
#include "shared/Protocol.h"
#include "shared/entities/Player.h"
#include "shared/types/Ids.h"

namespace client {
namespace {

constexpr uint32_t SHUTDOWN_DRAIN_MS = 50;

constexpr int SHUTDOWN_DRAIN_STEPS = 20;

} // namespace

ClientApp::ClientApp(uint16_t port, size_t peerCount)
    : ClientApp("127.0.0.1", port, peerCount) {
}

ClientApp::ClientApp(std::string hostName, uint16_t port, size_t peerCount)
    : Base(peerCount),
      hostName_(std::move(hostName)),
      port_(port) {
}

bool ClientApp::onStartUp() {
    std::cout << "Initiating connection to " << hostName_ << ":" << port_ << "...\n";

    peer_ = context_.connect(hostName_, port_);
    if (!peer_) {
        std::cerr << "ClientApp startup failed: Could not initiate connection.\n";
        return false;
    }

    renderer_.loadAtlas();

    return true;
}

void ClientApp::onTick(uint64_t /*tick*/) {
    if (WindowShouldClose()) {
        stop();
    }
    if (peer_ && peer_->state == ENET_PEER_STATE_CONNECTED) {
        if (const auto sample = world_.sampleIntent(tickSeconds())) {
            const auto& [seq, intent] = *sample;
            context_.send(peer_, shared::ControlsMessage{.seq = seq, .controls = intent.controls});
            for (const shared::Action& action : intent.actions) {
                std::visit([this](const auto& a) {
                    context_.send(peer_, shared::ActionMessage<std::remove_cvref_t<decltype(a)>>{a});
                }, action);
            }
        }
    }
    if (!mapReady_) {
        return;
    }
    auto& [cam, camCtrl] = world_.camera();
    if (world_.pawn() == shared::NULL_ENTITY) {
        return;
    }
    if (!boundReady_) {
        auto& map = world_.worldMap();
        constexpr float TILE = 16.0f;
        camCtrl.bounds = {0, 0, static_cast<float>(map.width()) * TILE, static_cast<float>(map.height()) * TILE};
        boundReady_ = true;
    }
    auto me = world_.entities().find<shared::Player>(world_.pawn());
    if (!me) {
        renderer_.render(world_.entities(), world_.worldMap(), cam, world_.pawn());
        return;
    }
    camCtrl.update(cam, tickSeconds(), me->body().pos, me->body().pos - me->prevPos());
    renderer_.render(world_.entities(), world_.worldMap(), cam, world_.pawn());
}

void ClientApp::onShutDown() {
    if (peer_ && peer_->state == ENET_PEER_STATE_CONNECTED) {
        enet_peer_disconnect(peer_, 0);

        for (int i = 0; i < SHUTDOWN_DRAIN_STEPS && peer_; ++i) {
            service(SHUTDOWN_DRAIN_MS);
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

void ClientApp::onReceive(ENetPeer* /*peer*/, ENetPacket* packet, uint8_t /*channelID*/) {
    const std::span<const std::byte> data(reinterpret_cast<const std::byte*>(packet->data), packet->dataLength);
    const auto result = shared::dispatch<shared::ServerMessages>(data, [this](const auto& message) {
        world_.apply(message);
        if constexpr (std::is_same_v<std::remove_cvref_t<decltype(message)>, shared::MapMessage>) {
            mapReady_ = true;
        }
    });
    if (result != shared::DispatchResult::OK) {
        //TODO: handle
        std::cout << "Bad packet (" << static_cast<int>(result) << ")" << std::endl;
    }
}

void ClientApp::onDisconnect(ENetPeer* /*peer*/) {
    std::cout << "Server disconnected. Terminating game loop.\n";
    peer_ = nullptr;
    stop();
}

} // namespace client
