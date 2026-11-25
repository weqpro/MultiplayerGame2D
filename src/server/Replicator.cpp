#include "server/Replicator.h"

#include <algorithm>
#include <unordered_set>

#include "shared/Protocol.h"

namespace server {

Replicator::Replicator(WebServerContext& net, uint32_t periodTicks)
    : net_{net}, periodTicks_{std::max<uint32_t>(periodTicks, 1)} {}

void Replicator::join(ENetPeer* peer, shared::EntityId pawn) {
    joiners_.push_back({peer, pawn});
}

void Replicator::leave(ENetPeer* peer) {
    std::erase_if(joiners_, [peer](const Joiner& joiner) { return joiner.peer == peer; });
}

void Replicator::flush(World& world, uint64_t tick) {
    replicateEvents(world);
    bootstrapJoiners(world);
    if (tick % periodTicks_ == 0) {
        sendSnapshot(world, tick);
    }
}

void Replicator::replicateEvents(World& world) {
    const World::Events events = world.drainEvents();
    const std::unordered_set<shared::EntityId> spawned(events.spawned.begin(), events.spawned.end());

    shared::SpawnMessage spawns;
    for (const shared::EntityId id : events.spawned) {
        if (const shared::Entity* entity = world.entities().find(id)) {
            spawns.entities.push_back(entity->describe());
        }
    }

    shared::DespawnMessage despawns;
    for (const shared::EntityId id : events.despawned) {
        if (!spawned.contains(id)) {
            despawns.ids.push_back(id);
        }
    }

    if (!spawns.entities.empty()) {
        net_.broadcast(spawns);
    }
    if (!despawns.ids.empty()) {
        net_.broadcast(despawns);
    }
    if (!events.attacks.empty()) {
        net_.broadcast(shared::AttackEventMessage{.attacks = std::move(events.attacks)});
    }
}

void Replicator::bootstrapJoiners(const World& world) {
    if (joiners_.empty()) {
        return;
    }

    shared::SpawnMessage msg;
    msg.entities.reserve(world.entities().size());
    for (const shared::Entity& entity : world.entities().all()) {
        msg.entities.push_back(entity.describe());
    }
    shared::MapMessage mapMsg {};
    auto& map = world.worldMap();
    mapMsg.map.height = map.height();
    mapMsg.map.width = map.width();
    mapMsg.map.layer.assign(map.layer().begin(), map.layer().end());

    for (const Joiner& joiner : joiners_) {
        net_.send(joiner.peer, shared::PossessMessage{.pawn = joiner.pawn});
        net_.send(joiner.peer, msg);
        net_.send(joiner.peer, mapMsg);
    }
    joiners_.clear();
}

void Replicator::sendSnapshot(const World& world, uint64_t tick) {
    shared::SnapshotMessage snapshot{.tick = static_cast<shared::Tick>(tick)};
    snapshot.entities.reserve(world.entities().size());
    for (const shared::Entity& entity : world.entities().all()) {
        snapshot.entities.push_back({.id = entity.id(), .body = entity.body()});
    }
    net_.broadcast(snapshot);
}

}
