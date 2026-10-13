#ifndef SHARED_TYPES_SPAWN_INFO_H
#define SHARED_TYPES_SPAWN_INFO_H

#include <string>
#include <tuple>

#include "shared/types/Appearance.h"
#include "shared/types/Body.h"
#include "shared/types/EntityKind.h"
#include "shared/types/Ids.h"

namespace shared {

struct SpawnInfo { // TODO: rename
    EntityId id{};
    EntityKind kind{};
    Body body{};
    Appearance look{};
    std::string name;

    static constexpr auto fields(auto& self) {
        return std::tie(self.id, self.kind, self.body, self.look, self.name);
    }
};

}

#endif
