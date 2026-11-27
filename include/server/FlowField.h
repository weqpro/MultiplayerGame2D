#ifndef SERVER_FLOW_FIELD_H
#define SERVER_FLOW_FIELD_H

#include <cstddef>
#include <vector>

#include <raylib.h>

#include "server/OccupancyMap.h"
#include "shared/WorldMap.h"

namespace server {

class FlowField {
public:
    FlowField() = default;
    FlowField(size_t w, size_t h);

    void compute(const shared::MapView& map, const OccupancyMap& occupancy,
                 int goalX, int goalY, float occupancyWeight);

    Vector2 direction(int x, int y) const;
    float cost(int x, int y) const;

    int goalX() const { return goalX_; }
    int goalY() const { return goalY_; }

private:
    size_t w_ = 0, h_ = 0;
    int goalX_ = -1, goalY_ = -1;
    std::vector<float> costField_;
    std::vector<Vector2> dirField_;
};

}

#endif
