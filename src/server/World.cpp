#include "server/World.h"
#include <raylib.h>

namespace server {

void World::update(float dt) {
    for (auto& player : players_) {
        player->update();
    }
    for (auto& player: players_) {
        player->body.pos.x += player->intent.input.x * player->body.speed * dt;
        player->body.pos.y += player->intent.input.y * player->body.speed * dt;
    }
}

size_t World::joinPlayer(Player *player) {
    players_.push_back(player);
    return players_.size() - 1;
}

void World::removePlayer(size_t playerId) {
    delete players_[playerId];
    players_.erase(players_.begin() + playerId);
}

void World::setPlayerIntent(size_t playerId, Intent intent) {
    players_[playerId]->intent = intent;
}


}
