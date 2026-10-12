#ifndef SHARED_RENDERER_H
#define SHARED_RENDERER_H

#include <unordered_map>
#include <cstdint>
#include <memory>

#include "shared/entities/Entity.h"

namespace shared {

class Renderer {
public:
    explicit Renderer(auto& entities) : entities_{entities} {};
    void render();
    void render(Body& body);
private:
    std::unordered_map<uint32_t, std::unique_ptr<shared::Entity>>& entities_;
};

}

#endif
