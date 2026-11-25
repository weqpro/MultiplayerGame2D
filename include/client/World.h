#ifndef CLIENT_WORLD_H
#define CLIENT_WORLD_H

#include <optional>
#include <utility>
#include <vector>

#include <raylib.h>

#include "shared/WorldMap.h"
#include "shared/Protocol.h"
#include "shared/controllers/CameraController.h"
#include "shared/controllers/KeyboardController.h"
#include "shared/entities/EntityRegistry.h"
#include "shared/types/Ids.h"
#include "shared/types/Intent.h"

namespace client {

struct ActiveAttack {
    shared::EntityId attacker;
    Vector2 origin;
    Vector2 dir;
    float timeLeft;
};

class World {
public:
    void apply(const shared::PossessMessage& message);
    void apply(const shared::SpawnMessage& message);
    void apply(const shared::DespawnMessage& message);
    void apply(const shared::SnapshotMessage& message);
    void apply(const shared::MapMessage& message);
    void apply(const shared::AttackEventMessage& message);

    void tickVisuals(float dt);

    [[nodiscard]] std::optional<std::pair<shared::InputSeq, shared::Intent>> sampleIntent(float dt);

    [[nodiscard]] const shared::EntityRegistry& entities() const noexcept { return registry_; }
    [[nodiscard]] shared::EntityId pawn() const noexcept { return pawn_; }
    [[nodiscard]] const shared::WorldMap& worldMap() const noexcept { return worldMap_; }
    [[nodiscard]] CameraWrapper& camera() noexcept { return cam_; }
    [[nodiscard]] const CameraWrapper& camera() const noexcept { return cam_; }
    [[nodiscard]] const std::vector<ActiveAttack>& activeAttacks() const noexcept { return activeAttacks_; }

private:
    shared::EntityRegistry registry_;
    shared::EntityId pawn_{shared::NULL_ENTITY};
    shared::KeyboardController input_;
    shared::InputSeq nextSeq_{1};
    std::optional<shared::Tick> lastTick_;
    shared::WorldMap worldMap_{};
    shared::MapView mapView_{worldMap_};
    CameraWrapper cam_;
    std::vector<ActiveAttack> activeAttacks_;

};

}

#endif // CLIENT_WORLD_H
