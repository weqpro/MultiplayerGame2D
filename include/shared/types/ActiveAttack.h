#ifndef SHARED_TYPES_ACTIVE_ATTACK_H
#define SHARED_TYPES_ACTIVE_ATTACK_H

#include <raylib.h>

#include "shared/types/Ids.h"

namespace shared {

struct ActiveAttack {
    EntityId attacker;
    Vector2 origin;
    Vector2 dir;
    float timeLeft;
    float radius{10.0f};
};

}

#endif
