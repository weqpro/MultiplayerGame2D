#ifndef SHARED_TYPES_INTENT_H
#define SHARED_TYPES_INTENT_H

#include <raylib.h>
#include <tuple>

namespace shared {

struct Intent {
    Vector2 move{};

    static constexpr auto fields(auto& self) { return std::tie(self.move); }
};

[[nodiscard]] Intent sanitize(Intent intent) noexcept;

}

#endif
