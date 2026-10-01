#include "client/World.h"
#include "client/Player.h"
#include <raylib.h>
#include <iostream>

#include "shared/Serialization.h"
#include "shared/Messages.h"

namespace client {

void World::update(float dt) {
    auto intent = playerController_.getIntent();

    shared::Packet<shared::IntentMessage> packet {
        .message = {intent.input}
    };

    auto bytes = packet.serialize();

    if (!bytes) {
        //TODO: handle
        std::cerr << "Intent message serialize failed" << std::endl;
        return;
    }

    messageBuf = bytes.value();
}

void World::joinPlayer(Player *player, size_t playerId) {
    players_[playerId] = player;
}

void World::removePlayer(size_t playerId) {
    delete players_[playerId];
    players_.erase(playerId);
}

}
