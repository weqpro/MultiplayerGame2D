#ifndef SHARED_ENTITIES_AGENT_H
#define SHARED_ENTITIES_AGENT_H

#include "shared/entities/Entity.h"
#include "shared/types/Intent.h"
#include "shared/types/Units.h"
#include "server/CollisionQuery.h"
#include "shared/types/WorldView.h"

namespace shared {

struct Locomotion {
    float speed{50.0f};
};

struct Combat {
    HealthPoint damage{10_hp};
    float reach{5.0f};     // edge-to-edge distance
    float cooldown{1.0f};  // seconds between attacks
};

class Agent : public Entity {
public:
    void act(const Intent& intent, const server::CollisionQuery& colQuery, WorldView& worldView);

    [[nodiscard]] const Locomotion& locomotion() const noexcept { return locomotion_; }
    [[nodiscard]] const Combat& combat() const noexcept { return combat_; }

    void takeDamage(HealthPoint damage) noexcept;

    [[nodiscard]] bool canAttack() const noexcept { return cooldownLeft_ <= 0.0f; }
    [[nodiscard]] bool inReach(const Entity& target) const noexcept;

protected:
    void resolve(const MeleeAction& action, WorldView& ctx);
    void resolve(const ShootAction& action, WorldView& ctx);
    void resolve(const PickUpAction& action, WorldView& ctx);

    Agent(EntityId id, const Body& body, const Appearance& look, const Locomotion& locomotion,  const Combat& combat)
        : Entity{id, body, look}, locomotion_{locomotion}, combat_{combat} {}

    virtual void doAct(const Controls& controls, const server::CollisionQuery& colQuery, WorldView& ctx);

private:
    Locomotion locomotion_;
    Combat combat_;
    float cooldownLeft_{0.0f};

    void startCooldown() noexcept { cooldownLeft_ = combat_.cooldown; }
};

}

#endif
