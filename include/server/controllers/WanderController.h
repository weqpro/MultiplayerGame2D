#ifndef SERVER_CONTROLLERS_WANDER_CONTROLLER_H
#define SERVER_CONTROLLERS_WANDER_CONTROLLER_H

#include <raylib.h>

#include "shared/contracts/IController.h"

namespace server {

class WanderController final : public shared::IController {
public:
    WanderController(Vector2 home, float leash) : home_{home}, leash_{leash} {}

    [[nodiscard]] shared::Intent decide(const shared::ControlContext& ctx) override;

private:
    Vector2 home_;
    float leash_;
    Vector2 heading_{};
    float untilTurn_{0.0f};
};

}

#endif
