#ifndef WORLD__H_
#define WORLD__H_

#include <optional>

#include <raylib.h>

#include "client/Player.h"
#include "client/WebClientContext.h"

namespace client {

class World {
public:
    void update(float dt);
    void joinPlayer(Vector2 spawn);

    World(WebClientContext &wctx) : wctx_(wctx) {}
private:
    Controller controller_ {};
    std::optional<Player> player_ = std::nullopt;
    WebClientContext &wctx_;
    uint8_t controllerId_ = 1;
};

}

#endif
