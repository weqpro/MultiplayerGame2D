#include "shared/entities/Agent.h"

#include <algorithm>

#include <raymath.h>

namespace shared {

void Agent::act(const Intent& intent, float dt) {
    cooldownLeft_ = std::max(0.0f, cooldownLeft_ - dt);
    doAct(sanitize(intent.controls), dt);
}

void Agent::doAct(const Controls& controls, float dt) {
    Body next = body();
    next.pos.x += controls.move.x * locomotion_.speed * dt;
    next.pos.y += controls.move.y * locomotion_.speed * dt;
    setBody(next);
}

bool Agent::inReach(const Entity& target) const noexcept {
    const float gap = Vector2Distance(body().pos, target.body().pos)
                    - appearance().radius - target.appearance().radius;
    return gap <= combat_.reach;
}

void Agent::takeDamage(HealthPoint damage) noexcept {
    Body next = body();
    next.health = std::max(0_hp, next.health - damage);
    setBody(next);
}

}
