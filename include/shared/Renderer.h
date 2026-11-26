#ifndef SHARED_RENDERER_H
#define SHARED_RENDERER_H

#include <span>

#include "shared/WorldMap.h"
#include "shared/entities/EntityRegistry.h"
#include "shared/types/ActiveAttack.h"
#include "shared/types/Ids.h"
#include <raylib.h>

namespace shared {

class Renderer {
public:
    static constexpr size_t TILE = 16;
    void render(const EntityRegistry& entities, const WorldMap& map, Camera2D camera,
                EntityId localPawn = NULL_ENTITY,
                std::span<const ActiveAttack> attacks = {},
                int pingMs = -1) const;
    void renderMap(const WorldMap& map, const Rectangle& view) const;
    void loadAtlas(const std::filesystem::path& texturePath = "assets/textures.png");
private:
    void drawEntity(const Entity& entity, bool local) const;
    void drawHealthBar(Vector2 entityPos, Vector2 entitySize, float health) const;
    void drawAttackArc(const ActiveAttack& attack) const;
    Texture2D atlas_;
    Rectangle source(int index) const;
    static inline Rectangle CameraWorldRect(const Camera2D& cam) {
        Vector2 tl = GetScreenToWorld2D({0, 0}, cam);
        Vector2 br = GetScreenToWorld2D({(float)GetScreenWidth(), (float)GetScreenHeight()}, cam);
        return { tl.x, tl.y, br.x - tl.x, br.y - tl.y };
    }
};

}

#endif
