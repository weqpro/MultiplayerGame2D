#ifndef SHARED_ENTITIES_PLAYER_H
#define SHARED_ENTITIES_PLAYER_H

#include <string>
#include <string_view>
#include <utility>
#include <concepts>

#include "shared/entities/Agent.h"

namespace shared {

class Player final : public Agent {
public:
    static constexpr EntityKind KIND = EntityKind::PLAYER;
    static constexpr Locomotion LOCOMOTION{.speed = 67.0f};
    static constexpr Combat COMBAT{.damage = 25_hp, .reach = 20.0f, .cooldown = 0.4f};

    Player(EntityId id, const Body& body, const Appearance& look,
           std::convertible_to<std::string> auto&& username)
      : Agent{id, body, look, LOCOMOTION, COMBAT},
        username_(std::forward<decltype(username)>(username)),
        prevPos_(body.pos) {}

    [[nodiscard]] EntityKind kind() const noexcept override { return KIND; }
    [[nodiscard]] std::string_view label() const noexcept override { return username_; }

    [[nodiscard]] const std::string& username() const noexcept { return username_; }
    [[nodiscard]] Vector2 prevPos() const noexcept { return prevPos_; }
    void savePrevPos() noexcept { prevPos_ = body().pos; }

private:
    void doAct(const Controls& controls, const server::CollisionQuery& colQuery, WorldView& ctx) override {
        prevPos_ = body().pos;
        Agent::doAct(controls, colQuery, ctx);
    }

    std::string username_;
    Vector2 prevPos_ {};
};

}

#endif
