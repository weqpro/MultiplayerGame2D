#ifndef SHARED_ENTITIES_PLAYER_H
#define SHARED_ENTITIES_PLAYER_H

#include <string>
#include <raylib.h>

#include "shared/entities/Agent.h"
#include "shared/PlayerController.h"
namespace shared {

class Player : public Agent {
public:
    explicit Player(std::unique_ptr<PlayerController> ctrl, Vector2 spawnPoint) : Agent{std::move(ctrl), spawnPoint} {}
    std::string username;
};

}

#endif
