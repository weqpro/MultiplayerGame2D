#ifndef SHARED_TYPES_APPEARANCE_H
#define SHARED_TYPES_APPEARANCE_H

#include <raylib.h>
#include <tuple>

namespace shared {

struct Appearance {
    Color color{BLACK};
    Vector2 size{12.0f, 12.0f};

    static constexpr auto fields(auto& self) { return std::tie(self.color, self.size); }
};

}

#endif
