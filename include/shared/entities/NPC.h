#ifndef SHARED_ENTITIES_NPC_H
#define SHARED_ENTITIES_NPC_H

#include "shared/entities/Agent.h"

namespace shared {

class NPC final : public Agent {
public:
    static constexpr EntityKind KIND = EntityKind::NPC;
    static constexpr Locomotion LOCOMOTION{.speed = 35.0f};

    NPC(EntityId id, const Body& body, const Appearance& look)
        : Agent{id, body, look, LOCOMOTION} {}

    [[nodiscard]] EntityKind kind() const noexcept override { return KIND; }
};

}

#endif
