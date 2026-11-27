#include "shared/entities/Agent.h"
#include "server/CollisionQuery.h"

#include <algorithm>
#include <variant>

#include <raymath.h>

#include "shared/Protocol.h"
#include "shared/entities/EntityRegistry.h"
#include "shared/types/EntityKind.h"

namespace shared {
namespace {

constexpr float MELEE_ARC_COS = 0.866f;

} // namespace

void Agent::act(const Intent& intent, const server::CollisionQuery& colQuery, WorldView& ctx) {
    cooldownLeft_ = std::max(0.0f, cooldownLeft_ - ctx.dt);
    doAct(sanitize(intent.controls), colQuery, ctx);
    for (const Action& action : intent.actions) {
        std::visit([this, &ctx](const auto& a) { resolve(a, ctx); }, action);
    }
}

void Agent::doAct(const Controls& controls, const server::CollisionQuery& colQuery, WorldView& ctx) {
    Body next = body();
    Vector2 delta = {
        controls.move.x * locomotion_.speed * ctx.dt,
        controls.move.y * locomotion_.speed * ctx.dt
    };
    server::AABB aabb = {
        {body().pos.x - appearance().size.x / 2, body().pos.y - appearance().size.y / 2},
        {body().pos.x + appearance().size.x / 2, body().pos.y + appearance().size.y / 2}
    };
    server::MoveResult res = colQuery.move(body().pos, aabb, delta, ctx);
    next.pos = res.lastValidPos;
    setBody(next);
}

void Agent::resolve(const MeleeAction& action, WorldView& ctx) {
    if (!canAttack()) {
        return;
    }
    startCooldown();

    if (ctx.attackEvents) {
        ctx.attackEvents->push_back({id(), body().pos, action.dir});
    }

    const Vector2 origin = body().pos;
    const Vector2 facing = Vector2Normalize(action.dir);
    for (const Agent& target : ctx.world.all<Agent>()) {
        switch (kind()) {
            case EntityKind::PLAYER:
                if (target.id() == id() || !inReach(target)) {
                    continue;
                }
                break;
            default:
                if (target.id() == id() || target.kind() == kind() || !inReach(target)) {
                    continue;
                }
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
    const float dx = std::abs(body().pos.x - target.body().pos.x);
    const float dy = std::abs(body().pos.y - target.body().pos.y);
    const float gapX = std::max(0.0f, dx - appearance().size.x / 2 - target.appearance().size.x / 2);
    const float gapY = std::max(0.0f, dy - appearance().size.y / 2 - target.appearance().size.y / 2);
    return gapX * gapX + gapY * gapY <= combat_.reach * combat_.reach;
}

void Agent::takeDamage(HealthPoint damage) noexcept {
    Body next = body();
    next.health = std::max(0_hp, next.health - damage);
    setBody(next);
}

}
