#include "server/World.h"
#include <raylib.h>
#include <vector>
#include <iostream>

#include "shared/Messages.h"

namespace server {

void World::update(float dt) {
    std::vector<shared::Player> players;
    for (auto& [playerId, player]: players_) {
        auto playerIntent = player->getIntent();
        player->body.pos.x += playerIntent.input.x * player->body.speed * dt;
        player->body.pos.y += playerIntent.input.y * player->body.speed * dt;
        players.push_back({
            .id = static_cast<uint32_t>(playerId), 
            .body = {
                .pos = player->body.pos
            }
        });
    };

    shared::Packet<shared::PlayersMessage> pack {
        .message = {
            .players = players
        }
    };
    auto bytes = pack.serialize();
    if (!bytes) {
        // TODO: handle
        std::cerr << "Could not serialize players" << std::endl;
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

void World::setClientPlayerIntent(size_t playerId, Intent intent) {
    players_[playerId]->setIntent(intent);
}


}
