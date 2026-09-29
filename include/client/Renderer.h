#ifndef RENDERER__H_
#define RENDERER__H_

#include <vector>

#include <client/Player.h>

namespace client {

class Renderer {
public:
    void render();
private:
    std::vector<Body*> bodies_;
};

}

#endif
