#include "client/World.h"
#include "client/Player.h"
#include "client/WebClientContext.h"
#include <raylib.h>
#include <string_view>

namespace client {

void World::update(float dt) {
    auto player = player_.value();
    player.update();

    std::string_view intentBytes {
        reinterpret_cast<const char*>(&player.intent),
        sizeof(Intent)
    };

    BeginDrawing();
    ClearBackground(RAYWHITE);
    player.body.render();
    EndDrawing();
}

void World::joinPlayer(Vector2 spawn) {
    player_.emplace(controller_, spawn);
}


}
