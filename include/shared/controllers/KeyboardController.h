#ifndef SHARED_CONTROLLERS_KEYBOARD_CONTROLLER_H
#define SHARED_CONTROLLERS_KEYBOARD_CONTROLLER_H

#include "shared/contracts/IController.h"

namespace shared {

class KeyboardController final : public IController {
public:
    [[nodiscard]] Intent decide(const WorldView& ctx) override;
};

}

#endif
