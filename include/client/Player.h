#ifndef PLAYER__H_
#define PLAYER__H_

#include <cmath>
#include <raylib.h>

namespace client {

struct Intent {
    Vector2 input {0.0, 0.0};
};

struct Body {
    Vector2 pos {NAN, NAN};
    float speed = 5.0f;
    void render() const;
};

class Controller {
public:
    Intent getIntent();
};

struct Player {
    Body body;
};

}

#endif
