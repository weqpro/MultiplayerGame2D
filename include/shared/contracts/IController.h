#ifndef SHARED_CONTRACTS_ICONTROLLER_H
#define SHARED_CONTRACTS_ICONTROLLER_H

#include "shared/types/Intent.h"

namespace shared {

class IController {
public:
    virtual ~IController() = default;
    virtual const Intent& getIntent() { return intent_; };
    virtual void update() = 0;
protected:
    shared::Intent intent_;
};

}

#endif
