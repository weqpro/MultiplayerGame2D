#include "shared/types/Intent.h"

#include <cmath>

namespace shared {

Controls sanitize(Controls controls) noexcept {
    if (!std::isfinite(controls.move.x)) {
        controls.move.x = 0.0f;
    }
    if (!std::isfinite(controls.move.y)) {
        controls.move.y = 0.0f;
    }
    const float length = std::hypot(controls.move.x, controls.move.y);
    if (length > 1.0f) {
        controls.move.x /= length;
        controls.move.y /= length;
    }
    return controls;
}

}
