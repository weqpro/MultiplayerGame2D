#ifndef WORLD__H_
#define WORLD__H_

#include <raylib.h>
#include <vector>

#include "client/Player.h"

namespace client {

class World {
public:
    World() : playerController_ {} {};
    void update(float dt);

    std::vector<std::byte> messageBuf;
private:
    Controller playerController_ {};
};

}

#endif
