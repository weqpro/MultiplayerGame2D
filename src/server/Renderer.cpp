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

void Renderer::addBody(Body *body) {
    bodies_.push_back(body);
}

}
