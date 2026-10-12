#include "server/World.h"
#include <raylib.h>

#include "shared/entities/Agent.h"

namespace server {

void World::update(float dt) {
    for (auto& [entityId,entity]: entities_) {
        if (auto* agent = dynamic_cast<shared::Agent*>(entity.get())){
            agent->controller->update();
        }
    };
    for (auto& [entityId,entity]: entities_) {
        if (auto* agent = dynamic_cast<shared::Agent*>(entity.get())){
            auto intent = agent->controller->getIntent();
            agent->body.pos.x += intent.input.x * agent->body.speed * dt;
            agent->body.pos.y += intent.input.y * agent->body.speed * dt;
        }
    }
}

}
