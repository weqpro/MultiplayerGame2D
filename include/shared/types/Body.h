#ifndef SHARED_TYPES_BODY_H
#define SHARED_TYPES_BODY_H

#include <raylib.h>
#include <tuple>

#include "shared/types/Units.h"

namespace shared {

struct Body {
    Vector2 pos{};
    health_point health{100_hp};

    static constexpr auto fields(auto& self) { return std::tie(self.pos, self.health); }
};

}

#endif
