#include "shared/types/Intent.h"

#include <cmath>

namespace shared {

Intent sanitize(Intent intent) noexcept {
    if (!std::isfinite(intent.move.x)) {
        intent.move.x = 0.0f;
    }
    if (!std::isfinite(intent.move.y)) {
        intent.move.y = 0.0f;
    }
    const float length = std::hypot(intent.move.x, intent.move.y);
    if (length > 1.0f) {
        intent.move.x /= length;
        intent.move.y /= length;
    }
    return intent;
}

}
