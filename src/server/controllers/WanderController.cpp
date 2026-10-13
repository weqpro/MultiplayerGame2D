#include "server/controllers/WanderController.h"

#include <cmath>
#include <numbers>

#include "shared/entities/Agent.h"

namespace server {
namespace {

constexpr float kReturnSeconds = 1.0f;

float randomSeconds(int minMs, int maxMs) {
    return static_cast<float>(GetRandomValue(minMs, maxMs)) / 1000.0f;
}

} // namespace

shared::Intent WanderController::decide(const shared::ControlContext& ctx) {
    const Vector2 pos = ctx.self.body().pos;
    const Vector2 toHome{home_.x - pos.x, home_.y - pos.y};
    const float distance = std::hypot(toHome.x, toHome.y);

    untilTurn_ -= ctx.dt;
    if (distance > leash_) {
        heading_ = {toHome.x / distance, toHome.y / distance};
        untilTurn_ = kReturnSeconds;
    } else if (untilTurn_ <= 0.0f) {
        untilTurn_ = randomSeconds(1000, 3000);
        if (GetRandomValue(0, 3) == 0) {
            heading_ = {};
        } else {
            const float angle = static_cast<float>(GetRandomValue(0, 359)) * std::numbers::pi_v<float> / 180.0f;
            heading_ = {std::cos(angle), std::sin(angle)};
        }
    }
    return {.move = heading_};
}

}
