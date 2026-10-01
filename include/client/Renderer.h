#ifndef CLIENT_RENDERER_H
#define CLIENT_RENDERER_H

#include <vector>

#include <client/Player.h>

namespace client {

class Renderer {
public:
    void render();
    void addBody(Body *body);
    std::vector<Body*> bodies_ {};
private:
};

}

#endif // CLIENT_RENDERER_H
