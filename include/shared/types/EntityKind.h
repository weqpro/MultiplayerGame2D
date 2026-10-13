#ifndef SHARED_TYPES_ENTITY_KIND_H
#define SHARED_TYPES_ENTITY_KIND_H

#include <cstdint>

namespace shared {

enum class EntityKind : std::uint8_t {
    PLAYER,
    NPC,
    COUNT
};

}

#endif
