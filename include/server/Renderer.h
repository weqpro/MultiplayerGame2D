#ifndef RENDERER__H_
#define RENDERER__H_

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

#endif
