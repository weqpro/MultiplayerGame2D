#include "client/World.h"
#include "client/Player.h"
#include <raylib.h>
#include <iostream>

#include "shared/Serialization.h"

namespace client {

void World::update(float dt) {
    auto intent = playerController_.getIntent();

    std::span<std::byte> buf{};
    shared::Writer writer(buf);
    writer.write(intent.input.x);
    writer.write(intent.input.y);

    if (!writer.ok()) {
        //TODO: handle
        std::cerr << "Writer not ok" << std::endl;
        std::cerr << "Written: " << writer.written() << std::endl;
    }

    messageBuf = buf;
}


}
