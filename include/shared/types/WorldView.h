#ifndef SHARED_TYPES_WORLD_VIEW_H
#define SHARED_TYPES_WORLD_VIEW_H
#include "shared/WorldMap.h"

namespace shared {

class Agent;
class EntityRegistry;

struct WorldView {
    Agent& self;
    EntityRegistry& world;
    MapView& map;
    const Camera2D& cam;
    float dt;
};

}

#endif
