#include "server/controllers/WanderController.h"

#include <cmath>

#include <raymath.h>

#include "shared/entities/Agent.h"

namespace server {
namespace {

constexpr int WAIT_PERCENT = 30;  // chance to stop before turning

float randomSeconds(int minMs, int maxMs) {
    return static_cast<float>(GetRandomValue(minMs, maxMs)) / 1000.0f;
}

// Random point on the edge of the (-1, -1) -> (1, 1) square.
Vector2 randomEdgePoint() {
    const float along = static_cast<float>(GetRandomValue(-100, 100)) / 100.0f;
    switch (GetRandomValue(0, 3)) {
        case 0:  return {along, -1.0f};  // top
        case 1:  return {1.0f, along};   // right
        case 2:  return {along, 1.0f};   // bottom
        default: return {-1.0f, along};  // left
    }
}

} // namespace

shared::Intent WanderController::decide(const shared::ControlContext& ctx) {
    const Vector2 pos = ctx.self.body().pos;
    const float dx = pos.x - home_.x;
    const float dy = pos.y - home_.y;
    const bool pastX = std::abs(dx) > leash_;
    const bool pastY = std::abs(dy) > leash_;

    untilTurn_ -= ctx.dt;

    const bool moving = heading_.x != 0.0f || heading_.y != 0.0f;
    // Past an edge and not heading back in.
    const bool bounce = moving && ((pastX && dx * heading_.x >= 0.0f)
                                || (pastY && dy * heading_.y >= 0.0f));

    if (bounce || untilTurn_ <= 0.0f) {
        if (moving && GetRandomValue(1, 100) <= WAIT_PERCENT) {
            heading_ = {};
            untilTurn_ = randomSeconds(4000, 5000);
        } else {
            heading_ = randomEdgePoint();
            // Point back inside on any axis that's past the leash.
            if (pastX) heading_.x = std::copysign(heading_.x, -dx);
            if (pastY) heading_.y = std::copysign(heading_.y, -dy);
            untilTurn_ = randomSeconds(2000, 3000);
        }
    }
    return {.move = Vector2Scale(Vector2Normalize(heading_), WANDER_SPEED_)};
}

}
