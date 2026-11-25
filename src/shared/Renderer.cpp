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

void Renderer::render(const EntityRegistry& entities, const WorldMap& map, Camera2D cam, EntityId localPawn) const {
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

    EndMode2D();
    EndDrawing();
}

void Renderer::drawEntity(const Entity& entity, bool local) const {
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
