#ifndef SERVER_COLLISION_QUERY_H
#define SERVER_COLLISION_QUERY_H

#include "shared/types/WorldView.h"
#include <raylib.h>
#include "shared/types/Ids.h"

namespace server {

struct AABB {
    Vector2 min;
    Vector2 max;

    Vector2 getHalves() const {
        return {
            (max.x - min.x)/2,  
            (max.y - min.y)/2,  
        };
    }

    Vector2 center() const {
        return {
            (max.x + min.x)/2,  
            (max.y + min.y)/2,  
        };
    }

    bool overlaps(const AABB& o) const {
    return min.x < o.max.x && max.x > o.min.x &&
            min.y < o.max.y && max.y > o.min.y;
    }

    static AABB fromCenter(Vector2 c, Vector2 halves) {
        return {{c.x - halves.x, c.y - halves.y}, {c.x + halves.x, c.y + halves.y}};
    }
};

struct MoveResult {
    bool hitX = false;
    bool hitY = false;
    Vector2 lastValidPos {};
    Vector2 normal;
};


struct Hit {
    enum class Kind : uint8_t {
        Tile,
        Entity,
        Count
    };
    bool hit = false;
    float time = 1;
    Vector2 normal;
    Kind kind;
    shared::EntityId entity;

};

class CollisionQuery {
public:
    MoveResult move(Vector2 pos, AABB& aabb, Vector2 delta, shared::WorldView& ctx) const;
private:
    Hit sweepTwo(const AABB& box, Vector2 delta, const AABB& other) const;
    Hit sweepRegion(const AABB& box, Vector2 delta, shared::WorldView& ctx) const;

    inline Vector2 minV(const Vector2 a, const Vector2 b) const { return { std::min(a.x, b.x), std::min(a.y, b.y) }; } 
    inline Vector2 maxV(Vector2 a, Vector2 b) const { return { std::max(a.x, b.x), std::max(a.y, b.y) }; }

    inline AABB sweptBounds(const AABB& box, Vector2 delta) const {
        return {
            minV(box.min, {box.min.x + delta.x, box.min.y + delta.y}),
            maxV(box.max, {box.max.x + delta.x, box.max.y + delta.y})
        };
    }
    static constexpr float INF = std::numeric_limits<float>::infinity();
};
}

#endif
