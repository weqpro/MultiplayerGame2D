#include "shared/Renderer.h"
#include <raylib.h>

namespace shared {

void Renderer::render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    for (auto& [entityId, entity] : entities_) {
        render(entity->body);
    }
    EndDrawing();
}

void Renderer::render(Body& body) {
    DrawCircleV(body.pos, 10.0f, body.color);
}

}
