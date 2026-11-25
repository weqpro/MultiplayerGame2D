#ifndef SERVER_CONTROLLERS_WANDER_CONTROLLER_H
#define SERVER_CONTROLLERS_WANDER_CONTROLLER_H

#include <raylib.h>

#include "shared/contracts/IController.h"

namespace server {

class WanderController : public shared::IController {
public:
    WanderController(Vector2 home, float leash) : home_{home}, leash_{leash} {} // TODO: add wander speed to ctor

    [[nodiscard]] shared::Intent decide(const shared::WorldView& ctx) override;

private:
    Vector2 home_;
    float leash_;
    Vector2 heading_{};
    float untilTurn_{0.0f};

    static constexpr float WANDER_SPEED_ = 0.5f;
};

}

#endif
