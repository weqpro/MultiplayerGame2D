#ifndef SERVER_RENDERER_H
#define SERVER_RENDERER_H

#include <vector>

#include "server/Player.h"

namespace server {

class Renderer {
public:
    void render();
    void addBody(Body *body);
    void removeBody(const Body *body);
private:
    std::vector<Body*> bodies_ {};
};

}

#endif // SERVER_RENDERER_H
