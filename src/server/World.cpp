#include "server/World.h"

#include <utility>

#include "shared/entities/NPC.h"
#include "shared/Renderer.h"

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
    occupancy_.clear();
    for (const shared::NPC& npc : registry_.all<shared::NPC>()) {
        int tx = int(npc.body().pos.x) / int(shared::Renderer::TILE);
        int ty = int(npc.body().pos.y) / int(shared::Renderer::TILE);
        occupancy_.mark(tx, ty);
    }
    flowFields_.update(registry_, mapView_, occupancy_, dt);

    decisions_.clear();
    for (auto& [id, possession] : possessions_) {
        shared::WorldView ctx{*possession.pawn, registry_, mapView_, cam_.camera, dt, nullptr, &flowFields_};
        decisions_.emplace_back(possession.pawn, possession.controller->decide(ctx));
    }
    for (auto& [pawn, intent] : decisions_) {
        shared::WorldView ctx{*pawn, registry_, mapView_, cam_.camera, dt, &events_.attacks, &flowFields_};
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

std::vector<shared::ActiveAttack> World::pendingActiveAttacks() const {
    std::vector<shared::ActiveAttack> result;
    for (const auto& info : events_.attacks) {
        float radius = 10.0f;
        if (const auto* agent = registry_.find<shared::Agent>(info.attacker)) {
            radius = agent->combat().reach + agent->appearance().size.x / 2.0f;
        }
        result.push_back({info.attacker, info.origin, info.dir, 0.15f, radius});
    }
    return result;
}

void World::setMap(const std::filesystem::path& mapPath) {
    for (size_t i = 0; i < LOAD_MAP_RETRY; ++i) {
        auto map = worldMap_.fromFile(mapPath);
        if (map) {
            worldMap_ = std::move(map.value());
            occupancy_ = OccupancyMap(worldMap_.width(), worldMap_.height());
            flowFields_ = FlowFieldManager(worldMap_.width(), worldMap_.height());
            return;
        }
    }
    DrawText("Failed to load map! Try buying a new PC", 20, 20, 20, RED);
}

}
