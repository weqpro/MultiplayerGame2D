#ifndef SHARED_RENDERER_H
#define SHARED_RENDERER_H

#include "shared/entities/EntityRegistry.h"
#include "shared/types/Ids.h"

namespace shared {

class Renderer {
public:
    void render(const EntityRegistry& entities, EntityId localPawn = NULL_ENTITY) const;

private:
    void draw(const Entity& entity, bool local) const;
};

}

#endif
