#include "server/World.h"

#include <utility>

namespace server {

namespace {
constexpr size_t LOAD_MAP_RETRY = 10; 
}

void World::despawn(shared::EntityId id) {
    possessions_.erase(id);
    if (registry_.erase(id)) {
        events_.despawned.push_back(id);
    }
}

void World::release(shared::EntityId pawn) {
    possessions_.erase(pawn);
}

void World::update(float dt) {
    decisions_.clear();
    for (auto& [id, possession] : possessions_) {
        shared::WorldView ctx{*possession.pawn, registry_, mapView_, cam_.camera, dt};
        decisions_.emplace_back(possession.pawn, possession.controller->decide(ctx));
    }
    for (auto& [pawn, intent] : decisions_) {
        shared::WorldView ctx{*pawn, registry_, mapView_, cam_.camera, dt};
        pawn->act(intent, colQuery_, ctx);
    }
    despawnDead();
}

void World::despawnDead() {
    std::vector<shared::EntityId> dead;
    for (const shared::Agent& agent : registry_.all<shared::Agent>()) {
        if (agent.body().health <= 0.0f) {
            dead.push_back(agent.id());
        }
    }
    for (const shared::EntityId id : dead) {
        despawn(id);
    }
}

World::Events World::drainEvents() {
    return std::exchange(events_, {});
}

void World::setMap(const std::filesystem::path& mapPath) {
    for (size_t i = 0; i < LOAD_MAP_RETRY; ++i) {
        auto map = worldMap_.fromFile(mapPath);
        if (map) {
            worldMap_ = std::move(map.value());
            return;
        }
    }
    DrawText("Failed to load map! Try buying a new PC", 20, 20, 20, RED);
}

}
