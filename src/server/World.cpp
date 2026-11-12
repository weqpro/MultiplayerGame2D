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
    std::vector<std::pair<shared::Agent*, shared::Intent>> decisions;
    decisions.reserve(possessions_.size());
    for (auto& [id, possession] : possessions_) {
        const shared::ControlContext ctx{*possession.pawn, registry_, dt};
        decisions.emplace_back(possession.pawn, possession.controller->decide(ctx));
    }
    for (auto& [pawn, intent] : decisions) {
        pawn->act(intent, dt);
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
