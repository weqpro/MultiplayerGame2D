#include "shared/controllers/KeyboardController.h"

#include <raylib.h>

namespace shared {

Intent KeyboardController::decide(const ControlContext& /*ctx*/) {
    Intent intent;
    if (IsKeyDown(KEY_W)) {
        intent.move.y -= 1.0f;
    }
    if (IsKeyDown(KEY_D)) {
        intent.move.x += 1.0f;
    }
    if (IsKeyDown(KEY_S)) {
        intent.move.y += 1.0f;
    }
    if (IsKeyDown(KEY_A)) {
        intent.move.x -= 1.0f;
    }
    return intent;
}

}
