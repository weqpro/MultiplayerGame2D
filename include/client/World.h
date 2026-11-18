#ifndef CLIENT_WORLD_H
#define CLIENT_WORLD_H

#include <optional>

#include "shared/WorldMap.h"
#include "shared/Protocol.h"
#include "shared/controllers/CameraController.h"
#include "shared/controllers/KeyboardController.h"
#include "shared/entities/EntityRegistry.h"
#include "shared/types/Ids.h"

namespace client {

class World {
public:
    void apply(const shared::PossessMessage& message);
    void apply(const shared::SpawnMessage& message);
    void apply(const shared::DespawnMessage& message);
    void apply(const shared::SnapshotMessage& message);
    void apply(const shared::MapMessage& message);

    [[nodiscard]] std::optional<shared::IntentMessage> sampleIntent(float dt);

    [[nodiscard]] const shared::EntityRegistry& entities() const noexcept { return registry_; }
    [[nodiscard]] shared::EntityId pawn() const noexcept { return pawn_; }
    [[nodiscard]] const shared::WorldMap& worldMap() const noexcept { return worldMap_; }
    [[nodiscard]] CameraWrapper& camera() noexcept { return cam_; }
    [[nodiscard]] const CameraWrapper& camera() const noexcept { return cam_; }

private:
    shared::EntityRegistry registry_;
    shared::EntityId pawn_{shared::NULL_ENTITY};
    shared::KeyboardController input_;
    shared::InputSeq nextSeq_{1};
    std::optional<shared::Tick> lastTick_;
    shared::WorldMap worldMap_{};
    CameraWrapper cam_;

};

}

#endif // CLIENT_WORLD_H
