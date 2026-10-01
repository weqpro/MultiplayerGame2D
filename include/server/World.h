#ifndef WORLD__H_
#define WORLD__H_

#include <unordered_map>
#include <vector>
#include <cstddef>

#include "server/Player.h"

namespace server {

class World {
public:
    void update(float dt);
    void joinPlayer(Player *player, size_t playerId);
    void removePlayer(size_t playerId);

    void setClientPlayerIntent(size_t playerId, Intent intent);
    std::vector<std::byte> messageBuf;
private:
    std::unordered_map<size_t, Player*> players_ {};
};

}

#endif
