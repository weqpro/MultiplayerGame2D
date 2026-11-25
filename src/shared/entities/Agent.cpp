#include "shared/entities/Agent.h"
#include "server/CollisionQuery.h"

namespace shared {

void Agent::act(const Intent& intent, float dt, const server::CollisionQuery& colQuery, WorldView& worldView) {
    doAct(sanitize(intent), dt, colQuery, worldView);
}

void Agent::doAct(const Intent& intent, float dt, const server::CollisionQuery& colQuery, WorldView& worldView) {
    Body next = body();
    prevPos = body().pos;
    Vector2 delta = {
        intent.move.x * locomotion_.speed * dt,
        intent.move.y * locomotion_.speed * dt
    };
    server::AABB aabb = {
        {body().pos.x - appearance().size.x / 2, body().pos.y - appearance().size.y / 2},
        {body().pos.x + appearance().size.x / 2, body().pos.y + appearance().size.y / 2}
    };
    server::MoveResult res = colQuery.move(body().pos, aabb, delta, worldView);
    setBody(next);
}

}
