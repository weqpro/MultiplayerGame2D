#include "server/World.h"
#include <raylib.h>

namespace server {

void World::update(float dt) {
    for (auto& player : players_) {
        player.update();
    }
    for (auto& player: players_) {
        player.body.pos.x += player.intent.input.x * player.body.speed * dt;
        player.body.pos.y += player.intent.input.y * player.body.speed * dt;
    }
    BeginDrawing();
    ClearBackground(RAYWHITE);
    for (auto& player : players_) {
        player.body.render();
    }
    EndDrawing();
}

void World::appendPlayer(Player player, Controller controller) {
    players_.push_back(player);
    controller_.push_back(controller);
}


}
