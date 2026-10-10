#ifndef SHARED_TYPES_INTENT_H
#define SHARED_TYPES_INTENT_H

#include <raylib.h>

#include "shared/Messages.h"

namespace shared {

struct Intent {
    Vector2 input {0.0, 0.0};
};

struct IntentMessage {
    static constexpr MessageType TYPE = MessageType::INTENT;
 
    Vector2 intent{};
 
    static constexpr auto fields(auto& self) { return std::tie(self.intent); }
};

}

#endif

