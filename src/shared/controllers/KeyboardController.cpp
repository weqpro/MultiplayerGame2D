#include "shared/controllers/KeyboardController.h"

#include <raylib.h>
#include <raymath.h>

#include "shared/entities/Agent.h"

namespace shared {

Intent KeyboardController::decide(const ControlContext& ctx) {
    Intent intent;
    if (IsKeyDown(KEY_W)) {
        intent.controls.move.y -= 1.0f;
    }
    if (IsKeyDown(KEY_D)) {
        intent.controls.move.x += 1.0f;
    }
    if (IsKeyDown(KEY_S)) {
        intent.controls.move.y += 1.0f;
    }
    if (IsKeyDown(KEY_A)) {
        intent.controls.move.x -= 1.0f;
    }
    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        const Vector2 toMouse = Vector2Subtract(GetMousePosition(), ctx.self.body().pos);
        intent.actions.push_back(MeleeAction{.dir = Vector2Normalize(toMouse)});
    }
    return intent;
}

}
