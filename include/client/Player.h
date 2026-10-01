#ifndef CLIENT_PLAYER_H
#define CLIENT_PLAYER_H

#include <cmath>
#include <raylib.h>

namespace client {

struct Intent {
    Vector2 input {0.0, 0.0};
};

struct Body {
    Vector2 pos {NAN, NAN};
    float speed = 5.0f;
    Color color = MAROON;
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

#endif // CLIENT_PLAYER_H
