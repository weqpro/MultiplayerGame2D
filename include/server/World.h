#ifndef WORLD__H_
#define WORLD__H_

#include <vector>

#include "server/Player.h"

namespace server {

class World {
public:
    void update(float dt);
    size_t joinPlayer(Player *player);
    void removePlayer(size_t playerId);

    void setClientPlayerIntent(size_t playerId, Intent intent);
private:
    std::vector<Player*> players_ {};
};

}

#endif
