#include "shared/Messages.h"

namespace shared {

void IntentMessage::serialize(Writer& writer) const {
    writer.write<float>(intent.x);
    writer.write<float>(intent.y);
}

}
