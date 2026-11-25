#ifndef SHARED_CONTRACTS_ICONTROLLER_H
#define SHARED_CONTRACTS_ICONTROLLER_H

#include "shared/types/Intent.h"
#include "shared/types/WorldView.h"

namespace shared {

class IController {
public:
    virtual ~IController() = default;

    [[nodiscard]] virtual Intent decide(const WorldView& ctx) = 0;
};

}

#endif
