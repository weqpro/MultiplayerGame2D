#include "server/controllers/RemotePlayerController.h"

namespace server{

bool RemotePlayerController::pushIntent(const shared::Intent& intent) {
    //TODO: validate intent
    intent_ = intent;
}
}
