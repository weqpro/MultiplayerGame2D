#ifndef SHARED_TYPES_INTENT_H
#define SHARED_TYPES_INTENT_H

#include <raylib.h>
#include <tuple>

#include "shared/types/Action.h"

namespace shared {

// Continuous input: only the latest value matters.
struct Controls {
    Vector2 move{};

    static constexpr auto fields(auto& self) { return std::tie(self.move); }
};

// Everything a controller decides in one tick. Never sent whole: see ControlsMessage and ActionMessage.
struct Intent {
    Controls controls;
    ActionList actions;
};

[[nodiscard]] Controls sanitize(Controls controls) noexcept;

}

#endif
