#include "client/World.h"
#include "client/Player.h"
#include "client/WebClientContext.h"
#include <raylib.h>
#include <cstddef>
#include <span>

namespace client {

void World::update(float dt) {
    auto player = player_.value();
    player.update();

    const std::span<const std::byte> intentBytes =
        std::as_bytes(std::span{&player.intent, 1});

    BeginDrawing();
    ClearBackground(RAYWHITE);
    player.body.render();
    EndDrawing();
}

void World::joinPlayer(Vector2 spawn) {
    player_.emplace(controller_, spawn);
}


}
