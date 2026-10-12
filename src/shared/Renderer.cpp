#include "shared/Renderer.h"

namespace shared {

void Renderer::render() {
    for (auto& [entityId, entity] : entities_) {
        render(entity->body);
    }
}

void Renderer::render(Body& body) {
    DrawCircleV(body.pos, 10.0f, body.color);
}

}
