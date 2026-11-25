#include "shared/entities/Agent.h"

#include <algorithm>
#include <variant>

#include <raymath.h>

#include "shared/entities/EntityRegistry.h"

namespace shared {
namespace {

constexpr float MELEE_ARC_COS = 0.5f;

} // namespace

void Agent::act(const Intent& intent, WorldView& ctx) {
    cooldownLeft_ = std::max(0.0f, cooldownLeft_ - ctx.dt);
    doAct(sanitize(intent.controls), ctx);
    for (const Action& action : intent.actions) {
        std::visit([this, &ctx](const auto& a) { resolve(a, ctx); }, action);
    }
}

void Agent::doAct(const Controls& controls, const WorldView& ctx) {
    Body next = body();
    next.pos.x += controls.move.x * locomotion_.speed * ctx.dt;
    next.pos.y += controls.move.y * locomotion_.speed * ctx.dt;
    setBody(next);
}

void Agent::resolve(const MeleeAction& action, WorldView& ctx) {
    if (!canAttack()) {
        return;
    }
    startCooldown();

    const Vector2 origin = body().pos;
    const Vector2 facing = Vector2Normalize(action.dir);
    for (const Agent& target : ctx.world.all<Agent>()) {
        if (target.id() == id() || target.kind() == kind() || !inReach(target)) {
            continue;
        }
        const Vector2 toTarget = Vector2Normalize(Vector2Subtract(target.body().pos, origin));
        if (Vector2DotProduct(facing, toTarget) < MELEE_ARC_COS) {
            continue;
        }
        if (auto* hit = ctx.world.find<Agent>(target.id())) {
            hit->takeDamage(combat().damage);
        }
    }
}

void Agent::resolve(const ShootAction& /*action*/, WorldView& /*ctx*/) {
    // TODO: projectiles
}

void Agent::resolve(const PickUpAction& /*action*/, WorldView& /*ctx*/) {
    // TODO: items
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
