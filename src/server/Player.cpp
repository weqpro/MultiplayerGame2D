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

Intent ClientController::getIntent() {
    return intent_;
}

void ClientController::setIntent(Intent i) {
    intent_ = i;
}

Intent Player::getIntent() {
    return controller_->getIntent();
}

void Player::setIntent(Intent i) {
    auto ctr = static_cast<ClientController *>(controller_.get());
    ctr->setIntent(i);
}
void Body::render() const {
    DrawCircleV(pos, 10.0f, LIME);
}

}
