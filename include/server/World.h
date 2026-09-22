#ifndef WORLD__H_
#define WORLD__H_

#include <vector>

#include "server/Player.h"

namespace server {

class World {
public:
    void update(float dt);
    void appendPlayer(Player player, Controller controller);
private:
    std::vector<Player> players_ {};
    std::vector<Controller> controller_ {};
};

}

#endif
