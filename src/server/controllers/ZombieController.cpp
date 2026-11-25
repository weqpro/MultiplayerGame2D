#include "server/controllers/ZombieController.h"

#include <optional>
#include <ranges>
#include <algorithm>
#include <vector>

#include <raymath.h>

#include "server/controllers/WanderController.h"
#include "shared/entities/Agent.h"
#include "shared/entities/Entity.h"
#include "shared/entities/EntityRegistry.h"
#include "shared/entities/Player.h"

namespace server {

shared::Intent ZombieController::decide(const shared::ControlContext& ctx) {
    const Vector2 selfPos = ctx.self.body().pos;

    const shared::Entity* target = targetId_ ? ctx.world.find(*targetId_) : nullptr;
    if (target && Vector2DistanceSqr(target->body().pos, selfPos) > playerDistanceSqr_) {
        target = nullptr;
    }

    if (!target) {
        targetId_ = tryFindPlayer(ctx);
        if (!targetId_) return WanderController::decide(ctx);
        target = ctx.world.find(*targetId_);
    }

    const Vector2 dir = Vector2Normalize(Vector2Subtract(target->body().pos, selfPos));
    shared::Intent intent{.controls = {.move = dir}};
    if (ctx.self.canAttack() && ctx.self.inReach(*target)) {
        intent.actions.push_back(shared::MeleeAction{.dir = dir});
    }
    return intent;
}

std::optional<shared::EntityId> ZombieController::tryFindPlayer(const shared::ControlContext& ctx) {
    const Vector2 selfPos = ctx.self.body().pos;

    auto nearby = ctx.world.all<shared::Player>()
                | std::views::transform([selfPos](const shared::Player& p) {
                    return std::pair{Vector2DistanceSqr(p.body().pos, selfPos), &p};
                })
                | std::views::filter([this](const auto& e) { return e.first <= playerDistanceSqr_; })
                | std::ranges::to<std::vector>();

    std::ranges::sort(nearby, {}, &decltype(nearby)::value_type::first);

    if (nearby.empty()) return std::nullopt;
    return nearby.front().second->id();
}

}
