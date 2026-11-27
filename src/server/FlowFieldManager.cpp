#include "server/FlowFieldManager.h"

#include <cmath>

#include "shared/entities/EntityRegistry.h"
#include "shared/entities/Player.h"
#include "shared/Renderer.h"

namespace server {

FlowFieldManager::FlowFieldManager(size_t mapW, size_t mapH, float occupancyWeight)
    : mapW_(mapW), mapH_(mapH), occupancyWeight_(occupancyWeight) {}

void FlowFieldManager::update(const shared::EntityRegistry& registry,
                               const shared::MapView& map,
                               const OccupancyMap& occupancy,
                               float dt) {
    for (const shared::Player& player : registry.all<shared::Player>()) {
        const int goalX = int(player.body().pos.x) / int(shared::Renderer::TILE);
        const int goalY = int(player.body().pos.y) / int(shared::Renderer::TILE);

        auto [it, inserted] = fields_.try_emplace(player.id());
        PlayerField& pf = it->second;

        if (inserted) {
            pf.field = FlowField(mapW_, mapH_);
        }

        pf.recomputeTimer -= dt;
        const bool goalChanged = (goalX != pf.lastGoalX || goalY != pf.lastGoalY);
        const bool timerExpired = (pf.recomputeTimer <= 0.0f);

        if (goalChanged || timerExpired) {
            pf.field.compute(map, occupancy, goalX, goalY, occupancyWeight_);
            pf.lastGoalX = goalX;
            pf.lastGoalY = goalY;
            pf.recomputeTimer = RECOMPUTE_INTERVAL;
        }
    }

    std::erase_if(fields_, [&registry](const auto& pair) {
        return registry.find(pair.first) == nullptr;
    });
}

Vector2 FlowFieldManager::getDirection(shared::EntityId playerId, Vector2 pos) const {
    auto it = fields_.find(playerId);
    if (it == fields_.end()) return {0, 0};

    constexpr float T = float(shared::Renderer::TILE);
    float sx = pos.x / T - 0.5f;
    float sy = pos.y / T - 0.5f;
    int ix = int(std::floor(sx));
    int iy = int(std::floor(sy));
    float fx = sx - float(ix);
    float fy = sy - float(iy);

    const auto& f = it->second.field;
    Vector2 d00 = f.direction(ix, iy);
    Vector2 d10 = f.direction(ix + 1, iy);
    Vector2 d01 = f.direction(ix, iy + 1);
    Vector2 d11 = f.direction(ix + 1, iy + 1);

    Vector2 d = {
        d00.x * (1 - fx) * (1 - fy) + d10.x * fx * (1 - fy) + d01.x * (1 - fx) * fy + d11.x * fx * fy,
        d00.y * (1 - fx) * (1 - fy) + d10.y * fx * (1 - fy) + d01.y * (1 - fx) * fy + d11.y * fx * fy,
    };
    float len = std::sqrt(d.x * d.x + d.y * d.y);
    if (len < 1e-6f) return {0, 0};
    return {d.x / len, d.y / len};
}

const FlowField* FlowFieldManager::fieldFor(shared::EntityId playerId) const {
    auto it = fields_.find(playerId);
    if (it == fields_.end()) return nullptr;
    return &it->second.field;
}

}
