#ifndef SERVER_CONTROLLERS_ZOMBIE_CONTROLLER_H
#define SERVER_CONTROLLERS_ZOMBIE_CONTROLLER_H

#include <optional>

#include <raylib.h>

#include "server/controllers/WanderController.h"
#include "shared/types/Ids.h"

namespace server {

class ZombieController final : public WanderController {
public:
    ZombieController(Vector2 home, float leash, float playerDistance)
      : WanderController{home, leash},
        playerDistanceSqr_{playerDistance * playerDistance} {}

    [[nodiscard]] shared::Intent decide(const shared::ControlContext& ctx) override;

private:
    std::optional<shared::EntityId> tryFindPlayer(const shared::ControlContext& ctx);

    std::optional<shared::EntityId> targetId_;
    float playerDistanceSqr_;
};

}

#endif
