#include "shared/Renderer.h"

#include <string>

#include <raylib.h>

namespace shared {
namespace {

constexpr int LABEL_FONT_SIZE = 10;

constexpr float LABEL_GAP = 2.0f;

constexpr float LOCAL_RING_GAP = 3.0f;

} // namespace

void Renderer::render(const EntityRegistry& entities, EntityId localPawn) const {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    for (const Entity& entity : entities.all()) {
        draw(entity, entity.id() == localPawn);
    }
    EndDrawing();
}

void Renderer::draw(const Entity& entity, bool local) const {
    const Vector2 pos = entity.body().pos;
    const Appearance& look = entity.appearance();

    DrawCircleV(pos, look.radius, look.color);
    if (local) {
        DrawCircleLinesV(pos, look.radius + LOCAL_RING_GAP, look.color);
    }

    const std::string label{entity.label()};
    if (!label.empty()) {
        const int width = MeasureText(label.c_str(), LABEL_FONT_SIZE);
        const float top = pos.y - look.radius - LABEL_GAP - static_cast<float>(LABEL_FONT_SIZE);
        DrawText(label.c_str(), static_cast<int>(pos.x) - width / 2, static_cast<int>(top), LABEL_FONT_SIZE, DARKGRAY);
    }
}

}
