#ifndef SHARED_ENTITIES_PLAYER_H
#define SHARED_ENTITIES_PLAYER_H

#include <string>
#include <string_view>
#include <utility>

#include "shared/entities/Agent.h"

namespace shared {

class Player final : public Agent {
public:
    static constexpr EntityKind KIND = EntityKind::PLAYER;
    static constexpr Locomotion LOCOMOTION{.speed = 67.0f};
    static constexpr Combat COMBAT{.damage = 25_hp, .reach = 6.0f, .cooldown = 0.4f};

    Player(EntityId id, const Body& body, const Appearance& look, std::string username)
        : Agent{id, body, look, LOCOMOTION, COMBAT}, username_{std::move(username)} {}

    [[nodiscard]] EntityKind kind() const noexcept override { return KIND; }
    [[nodiscard]] std::string_view label() const noexcept override { return username_; }

    [[nodiscard]] const std::string& username() const noexcept { return username_; }

private:
    std::string username_;
};

}

#endif
