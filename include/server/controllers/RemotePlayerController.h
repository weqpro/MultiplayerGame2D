#ifndef SERVER_CONTROLLERS_REMOTE_PLAYER_CONTROLLER_H
#define SERVER_CONTROLLERS_REMOTE_PLAYER_CONTROLLER_H

#include "shared/PlayerController.h"
#include "shared/types/Intent.h"

namespace server {

class RemotePlayerController : public shared::PlayerController {
public:
    bool pushIntent(const shared::Intent& intent);
};

}

#endif
