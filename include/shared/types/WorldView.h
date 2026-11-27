#ifndef SHARED_TYPES_WORLD_VIEW_H
#define SHARED_TYPES_WORLD_VIEW_H
#include "shared/WorldMap.h"

#include <vector>

namespace server { class FlowFieldManager; }

namespace shared {

class Agent;
class EntityRegistry;
struct AttackEventInfo;

struct WorldView {
    Agent& self;
    EntityRegistry& world;
    MapView& map;
    const Camera2D& cam;
    float dt;
    std::vector<AttackEventInfo>* attackEvents = nullptr;
    const server::FlowFieldManager* flowFields = nullptr;
};

}

#endif
