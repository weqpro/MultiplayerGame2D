#ifndef PLAYER__H_
#define PLAYER__H_

#include <raylib.h>

namespace server {

struct Intent {
    Vector2 input {0.0, 0.0};
};

struct Body {
    Vector2 pos;
    float speed = 5.0f;
    void render();
};

class Controller {
public:
    Intent getIntent();
};

class Player {
public:
    Body body;
    Intent intent;

    explicit Player(Controller& controller, Vector2 spawnPoint) : body(spawnPoint), intent(), controller_(controller) {}
    void update();
private:
    Controller &controller_;
};

}

#endif
