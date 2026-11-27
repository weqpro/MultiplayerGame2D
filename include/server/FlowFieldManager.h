#ifndef SERVER_FLOW_FIELD_MANAGER_H
#define SERVER_FLOW_FIELD_MANAGER_H

#include <cstddef>
#include <unordered_map>

#include <raylib.h>

#include "server/FlowField.h"
#include "server/OccupancyMap.h"
#include "shared/types/Ids.h"

namespace shared {
class EntityRegistry;
class MapView;
}

namespace server {

class FlowFieldManager {
public:
    FlowFieldManager() = default;
    FlowFieldManager(size_t mapW, size_t mapH, float occupancyWeight = 3.0f);

    void update(const shared::EntityRegistry& registry,
                const shared::MapView& map,
                const OccupancyMap& occupancy,
                float dt);

    Vector2 getDirection(shared::EntityId playerId, Vector2 npcPixelPos) const;
    const FlowField* fieldFor(shared::EntityId playerId) const;

private:
    struct PlayerField {
        FlowField field;
        int lastGoalX = -1, lastGoalY = -1;
        float recomputeTimer = 0.0f;
    };

    size_t mapW_ = 0, mapH_ = 0;
    float occupancyWeight_ = 3.0f;
    static constexpr float RECOMPUTE_INTERVAL = 0.25f;
    std::unordered_map<shared::EntityId, PlayerField> fields_;
};

}

#endif
