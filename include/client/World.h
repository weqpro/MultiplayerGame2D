#ifndef CLIENT_WORLD_H
#define CLIENT_WORLD_H

#include <optional>
#include <utility>

#include "shared/Protocol.h"
#include "shared/controllers/KeyboardController.h"
#include "shared/entities/EntityRegistry.h"
#include "shared/types/Ids.h"
#include "shared/types/Intent.h"

namespace client {

class World {
public:
    void apply(const shared::PossessMessage& message);
    void apply(const shared::SpawnMessage& message);
    void apply(const shared::DespawnMessage& message);
    void apply(const shared::SnapshotMessage& message);

    [[nodiscard]] std::optional<std::pair<shared::InputSeq, shared::Intent>> sampleIntent(float dt);

    [[nodiscard]] const shared::EntityRegistry& entities() const noexcept { return registry_; }
    [[nodiscard]] shared::EntityId pawn() const noexcept { return pawn_; }

private:
    shared::EntityRegistry registry_;
    shared::EntityId pawn_{shared::NULL_ENTITY};
    shared::KeyboardController input_;
    shared::InputSeq nextSeq_{1};
    std::optional<shared::Tick> lastTick_;
};

}

#endif // CLIENT_WORLD_H
