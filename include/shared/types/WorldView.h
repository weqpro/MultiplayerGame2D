#ifndef SHARED_TYPES_WORLD_VIEW_H
#define SHARED_TYPES_WORLD_VIEW_H

namespace shared {

class Agent;
class EntityRegistry;

struct WorldView {
    Agent& self;
    EntityRegistry& world;
    float dt;
};

}

#endif
