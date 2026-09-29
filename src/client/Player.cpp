#include "client/Player.h"
#include <cmath>
#include <raylib.h>

namespace client {

Intent Controller::getIntent() {
    Intent intent;
    if (IsKeyDown(KEY_W)) {
        intent.input.y -= 1.0f;
    } 
    if (IsKeyDown(KEY_D)) {
        intent.input.x += 1.0f;
    }
    if (IsKeyDown(KEY_S)) {
        intent.input.y += 1.0f;
    }
    if (IsKeyDown(KEY_A)) {
        intent.input.x -= 1.0f;
    }
    return intent;
}


void Body::render() const {
    if (std::isnan(pos.x) || std::isnan(pos.y)) return;
    DrawCircleV(pos, 10.0f, MAROON);
}

}
