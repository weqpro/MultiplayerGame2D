#include "server/World.h"

#include <utility>

#include <raymath.h>

namespace server {
namespace {

constexpr float MELEE_ARC_COS = 0.5f;  // hits within ±60° of the swing direction

} // namespace

void World::despawn(shared::EntityId id) {
    possessions_.erase(id);
    if (registry_.erase(id)) {
        events_.despawned.push_back(id);
    }
}

void World::release(shared::EntityId pawn) {
    possessions_.erase(pawn);
}

void World::update(float dt) {
    decisions_.clear();
    for (auto& [id, possession] : possessions_) {
        const shared::ControlContext ctx{*possession.pawn, registry_, dt};
        decisions_.emplace_back(possession.pawn, possession.controller->decide(ctx));
    }
    for (auto& [pawn, intent] : decisions_) {
        pawn->act(intent, dt);
    }
    for (auto& [pawn, intent] : decisions_) {
        for (const shared::Action& action : intent.actions) {
            shared::visitAction(action, [this, pawn](const auto& a) { resolve(*pawn, a); });
        }
    }
    despawnDead();
}

void World::resolve(shared::Agent& actor, const shared::MeleeAction& action) {
    if (!actor.canAttack()) {
        return;
    }
    actor.startCooldown();

    const Vector2 origin = actor.body().pos;
    const Vector2 facing = Vector2Normalize(action.dir);
    for (const shared::Agent& target : registry_.all<shared::Agent>()) {
        if (target.id() == actor.id() || target.kind() == actor.kind() || !actor.inReach(target)) {
            continue;
        }
        const Vector2 toTarget = Vector2Normalize(Vector2Subtract(target.body().pos, origin));
        if (Vector2DotProduct(facing, toTarget) < MELEE_ARC_COS) {
            continue;
        }
        if (auto* hit = registry_.find<shared::Agent>(target.id())) {
            hit->takeDamage(actor.combat().damage);
        }
    }
}

void World::resolve(shared::Agent& /*actor*/, const shared::ShootAction& /*action*/) {
    // TODO: projectiles
}

void World::resolve(shared::Agent& /*actor*/, const shared::PickUpAction& /*action*/) {
    // TODO: items
}

void World::despawnDead() {
    std::vector<shared::EntityId> dead;
    for (const shared::Agent& agent : registry_.all<shared::Agent>()) {
        if (agent.body().health <= 0.0f) {
            dead.push_back(agent.id());
        }
    }
    for (const shared::EntityId id : dead) {
        despawn(id);
    }
}

World::Events World::drainEvents() {
    return std::exchange(events_, {});
}

}
