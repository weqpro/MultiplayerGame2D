#ifndef WORLD__H_
#define WORLD__H_

#include <span>

#include <raylib.h>

#include "client/Player.h"

namespace client {

class World {
public:
    World() : playerController_ {} {};
    void update(float dt);

    std::span<std::byte> messageBuf;
private:
    Controller playerController_;
};

}

#endif
