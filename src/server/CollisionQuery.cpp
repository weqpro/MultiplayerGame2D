#include "server/CollisionQuery.h"
#include <raylib.h>
#include <cmath>
#include "shared/Renderer.h"
#include "shared/types/WorldView.h"
#include "shared/entities/Agent.h"

namespace server {


Hit CollisionQuery::sweepTwo(const AABB& box, Vector2 delta, const AABB& other) const {
    const Vector2 h = box.getHalves();
    const Vector2 position = box.center();
    const Vector2 low = {
        other.min.x - h.x, other.min.y - h.y };
    const Vector2 high = {
        other.max.x + h.x,
        other.max.y + h.y
    };


    Vector2 stepsMin;
    Vector2 stepsMax;

    auto intersectAxis = [](float pos, float del, float lo, float hi) -> std::optional<Vector2> {
        if (del == 0.0f) {
            if (pos <= lo || pos >= hi) {
                return std::nullopt;
            }
            return Vector2{-INF, INF};
        }

        float stepsToReachLow = (lo - pos) / del;
        float stepsToReachHigh = (hi - pos) / del;
        return Vector2{
            std::min(stepsToReachLow, stepsToReachHigh),
            std::max(stepsToReachLow, stepsToReachHigh)
        };
    };

    auto xRange = intersectAxis(position.x, delta.x, low.x, high.x);
    if (!xRange) {
        return {};
    }

    auto yRange = intersectAxis(position.y, delta.y, low.y, high.y);
    if (!yRange) {
        return {};
    }

    const float tEnter = std::max(xRange->x, yRange->x);
    const float tExit  = std::min(xRange->y, yRange->y);
    if (tEnter >= tExit || tEnter < 0.0f || tEnter > 1.0f) return {};

    Hit r;
    r.hit = true;
    r.time   = tEnter;
    if (xRange->x > yRange->x) {
        r.normal = { delta.x > 0 ? -1.f : 1.f, 0.f };   // entered through a vertical face
    }
    else {
        r.normal = { 0.f, delta.y > 0 ? -1.f : 1.f };   // entered through a horizontal face
    }
    return r;
}

Hit CollisionQuery::sweepRegion(const AABB& box, Vector2 delta, shared::WorldView& ctx) const {
    Hit best;
    const AABB r = sweptBounds(box, delta);
    const float tileSize = static_cast<float>(shared::Renderer::TILE);

    auto consider = [&](const Hit& h) {
        if (h.hit && h.time < best.time) best = h;
    };

    const int x0 = int(std::floor(r.min.x / tileSize));
    const int x1 = int(std::floor(r.max.x / tileSize));
    const int y0 = int(std::floor(r.min.y / tileSize));
    const int y1 = int(std::floor(r.max.y / tileSize));

    for (int y = y0; y <= y1; ++y)
        for (int x = x0; x <= x1; ++x) {
            if (!ctx.map.isSolid(x, y)) continue;
            AABB tile{{x * tileSize, y * tileSize}, {(x + 1) * tileSize, (y + 1) * tileSize}};
            Hit h = sweepTwo(box, delta, tile);
            h.kind = Hit::Kind::Tile;
            consider(h);
        }

    for (const shared::Agent& other : ctx.world.all<shared::Agent>()) {
        if (other.id() == ctx.self.id()) continue;

        const AABB otherBox = {
            {other.body().pos.x - other.appearance().size.x / 2, other.body().pos.y - other.appearance().size.y / 2},
            {other.body().pos.x + other.appearance().size.x / 2, other.body().pos.y + other.appearance().size.y / 2}
        };
        if (!r.overlaps(otherBox)) continue;


        Hit h = sweepTwo(box, delta, otherBox);
        h.kind   = Hit::Kind::Entity;
        h.entity = other.id();
        consider(h);
    }

    return best;
}

MoveResult CollisionQuery::move(Vector2 pos, AABB& aabb, Vector2 delta, shared::WorldView& ctx) const {
    constexpr int MAX_ITER = 4;
    constexpr float GAP = 1e-3f;
    constexpr float EPS = 1e-8f;

    MoveResult res;
    res.lastValidPos = pos;
    for (int i = 0; i < MAX_ITER && (delta.x * delta.x + delta.y * delta.y) > EPS; ++i) {
        Hit h = sweepRegion(aabb, delta, ctx);

        if (!h.hit) {
            pos.x += delta.x;
            pos.y += delta.y;
            res.lastValidPos = pos;
            return res;
        }

        pos.x += delta.x * h.time + h.normal.x * GAP;
        pos.y += delta.y * h.time + h.normal.y * GAP;

        Vector2 halves = aabb.getHalves();
        aabb.min = {pos.x - halves.x, pos.y - halves.y};
        aabb.max = {pos.x + halves.x, pos.y + halves.y};

        Vector2 rest;
        rest.x = delta.x * (1.f - h.time);
        rest.y = delta.y * (1.f - h.time);
        float dot = rest.x * h.normal.x + rest.y * h.normal.y;
        rest.x -= h.normal.x * dot;
        rest.y -= h.normal.y * dot;
        delta = rest;

        res.lastValidPos = pos;
        res.normal = h.normal;
        if (h.normal.x != 0.f) res.hitX = true;
        if (h.normal.y != 0.f) res.hitY = true;
    }
    return res;
}

}
