#include "shared/Renderer.h"
#include "shared/WorldMap.h"

#include <chrono>
#include <string>
#include <cmath>

#include <raylib.h>
#include <thread>

namespace shared {
namespace {

constexpr int LABEL_FONT_SIZE = 10;

constexpr float LABEL_GAP = 2.0f;

constexpr float LOCAL_RING_GAP = 3.0f;

constexpr float HP_BAR_WIDTH  = 24.0f;
constexpr float HP_BAR_HEIGHT = 3.0f;
constexpr float HP_BAR_GAP    = 2.0f;
constexpr HealthPoint MAX_HEALTH = 100.0f;

constexpr float ATTACK_ARC_HALF_ANGLE = 30.0f;
constexpr float ATTACK_ARC_DURATION = 0.15f;

constexpr size_t LOAD_TEXTURE_RETRY_COUNT = 10;

constexpr int ATLAS_OFFSET_X = 17;
constexpr int ATLAS_OFFSET_Y = 19;
constexpr int ATLAS_STRIDE_X = 17;
constexpr int ATLAS_STRIDE_Y = 19;
constexpr int ATLAS_COLS = 43;

constexpr int GRASS_TILES[] = {441, 484, 394};

size_t clampIdx(float v) {
    return v < 0.0f ? 0 : static_cast<size_t>(v);
}

} // namespace

void Renderer::render(const EntityRegistry& entities, const WorldMap& map, Camera2D cam,
                      EntityId localPawn, std::span<const ActiveAttack> attacks, int pingMs) const {
    BeginDrawing();
    ClearBackground(RAYWHITE);
    if (!IsTextureValid(atlas_)) {
        DrawText("Failed to load textures! Try buying a new PC", 20, 20, 20, RED);
        EndDrawing();
        return;
    }

    cam.target.x = std::round(cam.target.x * cam.zoom) / cam.zoom;
    cam.target.y = std::round(cam.target.y * cam.zoom) / cam.zoom;
    Rectangle view = CameraWorldRect(cam);

    BeginMode2D(cam);
    renderMap(map, view);

    for (const Entity& entity : entities.all()) {
        drawEntity(entity, entity.id() == localPawn);
    }

    for (const auto& attack : attacks) {
        drawAttackArc(attack);
    }

    EndMode2D();

    DrawFPS(10, 10);
    if (pingMs >= 0) {
        DrawText(TextFormat("Ping: %d ms", pingMs), 10, 34, 20, DARKGRAY);
    }

    EndDrawing();
}

void Renderer::drawEntity(const Entity& entity, bool local) const {
    const Vector2 pos = entity.body().pos;
    const Appearance& look = entity.appearance();

    const Rectangle rect = {
        pos.x - look.size.x / 2, pos.y - look.size.y / 2,
        look.size.x, look.size.y
    };
    DrawRectangleRec(rect, look.color);
    if (local) {
        const Rectangle ring = {
            rect.x - LOCAL_RING_GAP, rect.y - LOCAL_RING_GAP,
            rect.width + LOCAL_RING_GAP * 2, rect.height + LOCAL_RING_GAP * 2
        };
        DrawRectangleLinesEx(ring, 1.0f, look.color);
    }

    const bool showHpBar = (entity.kind() == EntityKind::PLAYER) ||
                           (entity.body().health < MAX_HEALTH);
    float barOffset = 0.0f;
    if (showHpBar) {
        drawHealthBar(pos, look.size, entity.body().health);
        barOffset = HP_BAR_HEIGHT + HP_BAR_GAP;
    }

    const std::string label{entity.label()};
    if (!label.empty()) {
        const int width = MeasureText(label.c_str(), LABEL_FONT_SIZE);
        const float top = pos.y - look.size.y / 2 - LABEL_GAP - barOffset
                          - static_cast<float>(LABEL_FONT_SIZE);
        DrawText(label.c_str(), static_cast<int>(pos.x) - width / 2,
                 static_cast<int>(top), LABEL_FONT_SIZE, DARKGRAY);
    }
}

void Renderer::drawHealthBar(Vector2 entityPos, Vector2 entitySize, float health) const {
    const float ratio = std::clamp(health / MAX_HEALTH, 0.0f, 1.0f);

    Color barColor;
    if (ratio > 0.6f)      barColor = {0, 228, 48, 255};
    else if (ratio > 0.3f) barColor = {253, 249, 0, 255};
    else                   barColor = {230, 41, 55, 255};

    const float barX = entityPos.x - HP_BAR_WIDTH / 2;
    const float barY = entityPos.y - entitySize.y / 2 - HP_BAR_GAP - HP_BAR_HEIGHT;

    const Rectangle bg = {barX, barY, HP_BAR_WIDTH, HP_BAR_HEIGHT};
    DrawRectangleRec(bg, {40, 40, 40, 200});

    const Rectangle fg = {barX, barY, HP_BAR_WIDTH * ratio, HP_BAR_HEIGHT};
    DrawRectangleRec(fg, barColor);

    DrawRectangleLinesEx(bg, 0.5f, {0, 0, 0, 150});
}

void Renderer::drawAttackArc(const ActiveAttack& attack) const {
    const float angleDeg = atan2f(attack.dir.y, attack.dir.x) * (180.0f / PI);
    const float startAngle = angleDeg - ATTACK_ARC_HALF_ANGLE;
    const float endAngle = angleDeg + ATTACK_ARC_HALF_ANGLE;
    const float alpha = std::clamp(attack.timeLeft / ATTACK_ARC_DURATION, 0.0f, 1.0f);
    const float radius = 10.0f;
    const Color color = {255, 255, 255, static_cast<unsigned char>(180.0f * alpha)};
    DrawCircleSector(attack.origin, radius, startAngle, endAngle, 12, color);
}

void Renderer::renderMap(const WorldMap& map, const Rectangle& view) const {
    if (map.width() == 0 || map.height() == 0) return;

    size_t c0 = clampIdx(std::floor(view.x / TILE));
    size_t r0 = clampIdx(std::floor(view.y / TILE));
    size_t c1 = std::min(map.width()  - 1, clampIdx(std::floor((view.x + view.width)  / TILE)));
    size_t r1 = std::min(map.height() - 1, clampIdx(std::floor((view.y + view.height) / TILE)));

    for (size_t r = r0; r <= r1; ++r) {
        for (size_t c = c0; c <= c1; ++c) {
            uint16_t id = map.tile(c, r);
            int atlasIdx;
            switch (id) {
                case 0: atlasIdx = GRASS_TILES[(c * 7 + r * 13) % 3]; break;
                case 1: atlasIdx = 441; break;
                case 2: atlasIdx = 87;  break;
                case 3: atlasIdx = 87;  break;
                default: continue;
            }
            DrawTextureRec(atlas_, source(atlasIdx),
                            { (float)(c * TILE), (float)(r * TILE) }, WHITE);
        }
    }
}

Rectangle Renderer::source(int index) const {
    return { static_cast<float>(ATLAS_OFFSET_X + (index % ATLAS_COLS) * ATLAS_STRIDE_X),
             static_cast<float>(ATLAS_OFFSET_Y + (index / ATLAS_COLS) * ATLAS_STRIDE_Y),
             static_cast<float>(TILE), static_cast<float>(TILE) };
}

void Renderer::loadAtlas(const std::filesystem::path& texturePath) {
    for (size_t i = 0; i < LOAD_TEXTURE_RETRY_COUNT; ++i) {
        atlas_ = LoadTexture(texturePath.c_str());
        if (IsTextureValid(atlas_)) {
            return;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    TraceLog(LOG_ERROR, "Failed to load player texture!");
    DrawText("Failed to load textures! Try buying a new PC", 20, 20, 20, RED);
}

}
