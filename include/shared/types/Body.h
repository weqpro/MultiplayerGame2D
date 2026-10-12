#ifndef SHARED_TYPES_BODY_H
#define SHARED_TYPES_BODY_H

#include <raylib.h>
#include <tuple>

namespace shared {

struct Body {
    Vector2 pos{};
    Color color{};
    float speed{};

    static constexpr auto fields(auto& self) { return std::tie(self.pos, self.color); }
};

}

#endif

