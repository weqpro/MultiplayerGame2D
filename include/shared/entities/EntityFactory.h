#ifndef SHARED_ENTITIES_ENTITY_FACTORY_H
#define SHARED_ENTITIES_ENTITY_FACTORY_H

#include <memory>

#include "shared/entities/Entity.h"
#include "shared/types/SpawnInfo.h"

namespace shared {

[[nodiscard]] std::unique_ptr<Entity> makeEntity(const SpawnInfo& info);

}

#endif
