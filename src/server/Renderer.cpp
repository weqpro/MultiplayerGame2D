#include "server/Renderer.h"

namespace server {

void Renderer::render() {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    for (auto& body: bodies_) {
        body->render();
    }
    EndDrawing();
}

}
