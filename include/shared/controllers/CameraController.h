#ifndef SHARED_CAMERA_CONTROLLER_H
#define SHARED_CAMERA_CONTROLLER_H

#include "raylib.h"
#include "raymath.h"
#include <cmath>

//TODO: make child of IController?
class CameraController {
public:
    Rectangle bounds{};
    Vector2   deadHalf{60, 40};
    float     lookAhead = 0.2f;
    float     stiffness = 8.0f;

    void snapTo(Camera2D& cam, Vector2 t) const {
        cam.offset = ScreenCenter();
        cam.target = Clamp(cam, t);
    }

    void update(Camera2D& cam, float dt, Vector2 playerPos, Vector2 playerVel) const {
        cam.offset = ScreenCenter();

        Vector2 p = Vector2Add(playerPos, Vector2Scale(playerVel, lookAhead));

        Vector2 goal = cam.target;
        if      (p.x < cam.target.x - deadHalf.x) goal.x = p.x + deadHalf.x;
        else if (p.x > cam.target.x + deadHalf.x) goal.x = p.x - deadHalf.x;
        if      (p.y < cam.target.y - deadHalf.y) goal.y = p.y + deadHalf.y;
        else if (p.y > cam.target.y + deadHalf.y) goal.y = p.y - deadHalf.y;

        float t = 1.0f - std::exp(-stiffness * dt);
        cam.target = Vector2Lerp(cam.target, goal, t);

        cam.target = Clamp(cam, cam.target);
    }

private:
    static Vector2 ScreenCenter() {
        return { GetScreenWidth() * 0.5f, GetScreenHeight() * 0.5f };
    }

    Vector2 Clamp(const Camera2D& cam, Vector2 t) const {
        float halfW = cam.offset.x / cam.zoom;
        float halfH = cam.offset.y / cam.zoom;

        float minX = bounds.x + halfW, maxX = bounds.x + bounds.width  - halfW;
        float minY = bounds.y + halfH, maxY = bounds.y + bounds.height - halfH;

        t.x = (minX > maxX) ? bounds.x + bounds.width  * 0.5f : Clampf(t.x, minX, maxX);
        t.y = (minY > maxY) ? bounds.y + bounds.height * 0.5f : Clampf(t.y, minY, maxY);
        return t;
    }

    static float Clampf(float v, float lo, float hi) { return v < lo ? lo : (v > hi ? hi : v); }
};

struct CameraWrapper {
    Camera2D camera;
    CameraController camCtrl;
};

#endif
