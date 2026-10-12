#ifndef SERVER_CONTROLLERS_LOCAL_PLAYER_CONTROLLER_H
#define SERVER_CONTROLLERS_LOCAL_PLAYER_CONTROLLER_H

#include "shared/PlayerController.h"
namespace server {

class LocalPlayerController : public shared::PlayerController {
public:
    void update() override;
};

}

#endif
