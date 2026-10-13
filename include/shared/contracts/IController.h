#ifndef SHARED_CONTRACTS_ICONTROLLER_H
#define SHARED_CONTRACTS_ICONTROLLER_H

#include "shared/types/Intent.h"

namespace shared {

class Agent;
class EntityRegistry;

struct ControlContext {
    const Agent& self;
    const EntityRegistry& world;
    float dt;
};

class IController {
public:
    virtual ~IController() = default;

    [[nodiscard]] virtual Intent decide(const ControlContext& ctx) = 0;
};

}

#endif
