#ifndef SHARED_TYPES_ACTION_H
#define SHARED_TYPES_ACTION_H

#include <cstddef>
#include <optional>
#include <tuple>
#include <type_traits>
#include <variant>

#include <raylib.h>

#include "shared/containers/FixedVector.h"
#include "shared/types/Ids.h"

namespace shared {

struct MeleeAction {
    Vector2 dir{};

    static constexpr auto fields(auto& self) { return std::tie(self.dir); }
};

struct ShootAction {
    Vector2 dir{};

    static constexpr auto fields(auto& self) { return std::tie(self.dir); }
};

struct PickUpAction {
    EntityId item{NULL_ENTITY};

    static constexpr auto fields(auto& self) { return std::tie(self.item); }
};

// Order = wire-id order of ActionMessage<A>. Client and server are always built from the same source.
using Action = std::variant<MeleeAction, ShootAction, PickUpAction>;

inline constexpr std::size_t MAX_ACTIONS_PER_TICK = 4;
using ActionList = FixedVector<Action, MAX_ACTIONS_PER_TICK>;

static_assert(std::is_trivially_copyable_v<Action>);
static_assert(sizeof(Action) <= 12, "Action grew; re-check Intent size");

// Rejects non-finite or zero directions and null items; normalizes directions.
[[nodiscard]] std::optional<Action> sanitize(const Action& action) noexcept;

}

#endif
