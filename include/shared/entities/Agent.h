#ifndef SHARED_ENTITIES_AGENT_H
#define SHARED_ENTITIES_AGENT_H

#include <memory>

#include "shared/entities/Entity.h"
#include "shared/contracts/IController.h"

namespace shared {

class Agent : public Entity {
public:
    explicit Agent(std::unique_ptr<IController> ctrl, Vector2 spawn) : Entity{spawn}, controller{std::move(ctrl)} {}
    std::unique_ptr<IController> controller;
};

}

#endif
