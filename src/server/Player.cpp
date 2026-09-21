#include "server/Player.h"
#include <raylib.h>

namespace server {

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

void Player::update() {
    intent = controller_.getIntent();
}

void Body::render() {
    DrawCircleV(pos, 10, MAROON);
}

}
