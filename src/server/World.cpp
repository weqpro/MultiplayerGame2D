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
                .pos = player->body.pos,
                .color = player->body.color
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
    auto it = players_.find(playerId);
    if (it == players_.end()) {
        return;
    }
    delete it->second;
    players_.erase(it);
}

Player* World::getPlayer(size_t playerId) {
    auto it = players_.find(playerId);
    return it == players_.end() ? nullptr : it->second;
}

void World::setClientPlayerIntent(size_t playerId, Intent intent) {
    if (Player* player = getPlayer(playerId)) {
        player->setIntent(intent);
    }
}


}
