#include "shared/entities/Agent.h"

namespace shared {

void Agent::act(const Intent& intent, float dt) {
    doAct(sanitize(intent), dt);
}

void Agent::doAct(const Intent& intent, float dt) {
    Body next = body();
    next.pos.x += intent.move.x * locomotion_.speed * dt;
    next.pos.y += intent.move.y * locomotion_.speed * dt;
    setBody(next);
}

}
