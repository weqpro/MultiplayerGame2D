#ifndef SHARED_ENTITIES_AGENT_H
#define SHARED_ENTITIES_AGENT_H

#include "shared/entities/Entity.h"
#include "shared/types/Intent.h"

namespace shared {

struct Locomotion {
    float speed{50.0f};
};

class Agent : public Entity {
public:
    void act(const Intent& intent, float dt);

    [[nodiscard]] const Locomotion& locomotion() const noexcept { return locomotion_; }

    Vector2 prevPos {};
protected:
    Agent(EntityId id, const Body& body, const Appearance& look, const Locomotion& locomotion)
        : Entity{id, body, look}, locomotion_{locomotion}, prevPos(body.pos) {}

    virtual void doAct(const Intent& intent, float dt);

private:
    Locomotion locomotion_;
};

}

#endif
