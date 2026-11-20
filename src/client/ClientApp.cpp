#include "client/ClientApp.h"

#include <cstddef>
#include <enet/enet.h>
#include <iostream>
#include <span>
#include <string>
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
        if (const auto intent = world_.sampleIntent(tickSeconds())) {
            context_.send(peer_, *intent);
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
    camCtrl.update(cam, tickSeconds(), me->body().pos, me->body().pos - me->prevPos);
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

template <shared::Message M>
void ClientApp::dispatch(std::span<const std::byte> data) {
    const auto packet = shared::Packet<M>::deserialize(data);
    if (!packet) {
        //TODO: handle
        std::cout << "Deserialize failed for message type " << static_cast<int>(M::TYPE) << std::endl;
        return;
    }
    world_.apply(packet->message);
}

void ClientApp::onReceive(ENetPeer* /*peer*/, ENetPacket* packet, uint8_t /*channelID*/) {
    const std::span<const std::byte> data(reinterpret_cast<const std::byte*>(packet->data), packet->dataLength);
    const auto type = shared::peek_type(data);
    if (!type) {
        //TODO: handle
        std::cout << "Invalid type" << std::endl;
        return;
    }
    switch (*type) {
        case shared::MessageType::POSSESS:
            dispatch<shared::PossessMessage>(data);
            break;
        case shared::MessageType::SPAWN:
            dispatch<shared::SpawnMessage>(data);
            break;
        case shared::MessageType::DESPAWN:
            dispatch<shared::DespawnMessage>(data);
            break;
        case shared::MessageType::SNAPSHOT:
            dispatch<shared::SnapshotMessage>(data);
            break;
        case shared::MessageType::MAP:
            dispatch<shared::MapMessage>(data);
            mapReady_ = true;
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
