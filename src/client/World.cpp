#include "client/World.h"

#include <iostream>
#include <utility>

#include "shared/entities/Agent.h"
#include "shared/entities/EntityFactory.h"

namespace client {

void World::apply(const shared::PossessMessage& message) {
    pawn_ = message.pawn;
}

void World::apply(const shared::SpawnMessage& message) {
    for (const shared::SpawnInfo& info : message.entities) {
        auto entity = shared::makeEntity(info);
        if (!entity) {
            std::cerr << "Cannot spawn entity " << info.id << " of kind "
                      << static_cast<int>(info.kind) << std::endl;
            continue;
        }
        registry_.insert(std::move(entity));
    }
}

void World::apply(const shared::DespawnMessage& message) {
    for (const shared::EntityId id : message.ids) {
        registry_.erase(id);
    }
}

void World::apply(const shared::SnapshotMessage& message) {
    if (lastTick_ && message.tick <= *lastTick_) {
        return;
    }
    lastTick_ = message.tick;
    for (const shared::EntityState& state : message.entities) {
        if (shared::Entity* entity = registry_.find(state.id)) {
            entity->setBody(state.body);
        }
    }
}

std::optional<shared::IntentMessage> World::sampleIntent(float dt) {
    const auto* pawn = registry_.find<shared::Agent>(pawn_);
    if (!pawn) {
        return std::nullopt;
    }
    return shared::IntentMessage{
        .seq = nextSeq_++,
        .intent = input_.decide({*pawn, registry_, dt}),
    };
}

}
