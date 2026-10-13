#include "shared/entities/Entity.h"

#include <string>

namespace shared {

SpawnInfo Entity::describe() const {
    return SpawnInfo{
        .id = id_,
        .kind = kind(),
        .body = body_,
        .look = look_,
        .name = std::string{label()},
    };
}

}
