#ifndef SHARED_ENTITIES_ENTITY_H
#define SHARED_ENTITIES_ENTITY_H

#include "shared/types/Body.h"
#include <raylib.h>

namespace shared {

class Entity {
public:
    Entity(Vector2 spawn) : body{spawn} {}
    virtual ~Entity() = default;
    Body body;
};

}

#endif
