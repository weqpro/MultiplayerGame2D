#include "server/World.h"
#include <raylib.h>

#include <iostream>

namespace server {

void World::update(float dt) {
    for (auto& player: players_) {
        auto playerIntent = player->getIntent();
        player->body.pos.x += playerIntent.input.x * player->body.speed * dt;
        player->body.pos.y += playerIntent.input.y * player->body.speed * dt;
    }
}

size_t World::joinPlayer(Player *player) {
    players_.push_back(player);
    size_t id = players_.size() - 1;
    std::cout << "Player joined: " << id << std::endl; 
    return id;
}

void World::removePlayer(size_t playerId) {
    delete players_[playerId];
    players_.erase(players_.begin() + playerId);
}

void World::setClientPlayerIntent(size_t playerId, Intent intent) {
    players_[playerId]->setIntent(intent);
}


}
