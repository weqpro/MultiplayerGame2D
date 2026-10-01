#ifndef PLAYER__H_
#define PLAYER__H_

#include <memory>

#include <raylib.h>

namespace server {

struct Intent {
    Vector2 input {0.0, 0.0};
};

struct Body {
    Vector2 pos {0.0f, 0.0f};
    float speed = 50.0f;
    Color color = DARKGRAY;
    void render() const;
};

Color randomPlayerColor();

class Controller {
public:
    virtual Intent getIntent();
};

class ClientController : public Controller {
public:
    Intent getIntent() override;
    void setIntent(Intent intent);
private:
    Intent intent_ {};
};

class Player {
public:
    Body body;

    explicit Player(std::unique_ptr<Controller> controller, Vector2 spawnPoint) : body(), controller_(std::move(controller)) {
        body.pos = spawnPoint;
        body.color = randomPlayerColor();
    }
    Intent getIntent();
    void setIntent(Intent i);
private:
    std::unique_ptr<Controller> controller_; //TODO: make server player controller
};

}

#endif
