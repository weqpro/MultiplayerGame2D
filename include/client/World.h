#ifndef CLIENT_WORLD_H
#define CLIENT_WORLD_H

#include <raylib.h>
#include <unordered_map>
#include <vector>

#include "client/Player.h"

namespace client {

class World {
public:
    World() : playerController_ {} {};
    void update(float dt);
    void joinPlayer(Player player, size_t playerId);
    void removePlayer(size_t playerId);

    std::vector<std::byte> messageBuf;
    std::unordered_map<size_t, Player> players_;
private:
    Controller playerController_ {};
};

}

#endif // CLIENT_WORLD_H
