#include "shared/Messages.h"

namespace shared {

std::span<const std::byte> IntentMessage::serialize() {
    for (size_t i = 0; i < sizeof(float)/sizeof(std::byte); ++i) {

    }
    return data;
}

}
