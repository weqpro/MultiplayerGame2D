#include "shared/types/Action.h"

#include <cmath>

namespace shared {
namespace {

std::optional<Vector2> sanitizeDir(Vector2 dir) noexcept {
    if (!std::isfinite(dir.x) || !std::isfinite(dir.y)) {
        return std::nullopt;
    }
    const float length = std::hypot(dir.x, dir.y);
    if (length == 0.0f) {
        return std::nullopt;
    }
    return Vector2{dir.x / length, dir.y / length};
}

std::optional<Action> sanitizeOne(MeleeAction action) noexcept {
    const auto dir = sanitizeDir(action.dir);
    return dir ? std::optional<Action>{MeleeAction{.dir = *dir}} : std::nullopt;
}

std::optional<Action> sanitizeOne(ShootAction action) noexcept {
    const auto dir = sanitizeDir(action.dir);
    return dir ? std::optional<Action>{ShootAction{.dir = *dir}} : std::nullopt;
}

std::optional<Action> sanitizeOne(PickUpAction action) noexcept {
    return action.item != NULL_ENTITY ? std::optional<Action>{action} : std::nullopt;
}

} // namespace

std::optional<Action> sanitize(const Action& action) noexcept {
    std::optional<Action> result;
    std::visit([&result](const auto& a) { result = sanitizeOne(a); }, action);
    return result;
}

}
