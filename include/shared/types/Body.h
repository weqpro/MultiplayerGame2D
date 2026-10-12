#ifndef SHARED_TYPES_BODY_H
#define SHARED_TYPES_BODY_H

#include <raylib.h>
#include <tuple>

namespace shared {

struct Body {
    Vector2 pos{};
    Color color {BLACK};
    float speed {50.0f};

    static constexpr auto fields(auto& self) { return std::tie(self.pos, self.color); }
};

}

#endif

