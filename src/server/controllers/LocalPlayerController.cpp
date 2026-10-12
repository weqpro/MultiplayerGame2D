#include "server/controllers/LocalPlayerController.h"

namespace server{

void LocalPlayerController::update() {
    if (IsKeyDown(KEY_W)) {
        intent_.input.y -= 1.0f;
    } 
    if (IsKeyDown(KEY_D)) {
        intent_.input.x += 1.0f;
    }
    if (IsKeyDown(KEY_S)) {
        intent_.input.y += 1.0f;
    }
    if (IsKeyDown(KEY_A)) {
        intent_.input.x -= 1.0f;
    }
}

}
