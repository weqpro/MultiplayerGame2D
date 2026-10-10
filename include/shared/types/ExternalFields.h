#ifndef SHARED_TYPES_EXTERNAL_FIELDS_H
#define SHARED_TYPES_EXTERNAL_FIELDS_H

#include <raylib.h>
#include "shared/Messages.h"

namespace shared {
template <>
struct ExternalFields<Vector2> {
    static constexpr auto get(auto& v) { return std::tie(v.x, v.y); }
};

template <>
struct ExternalFields<Color> {
    static constexpr auto get(auto& c) { return std::tie(c.r, c.g, c.b, c.a); }
};

}

#endif
