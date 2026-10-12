#ifndef SERVER_WORLD_H
#define SERVER_WORLD_H

#include <unordered_map>
#include <cstdint>
#include <memory>

#include "shared/entities/Entity.h"

namespace server {

class World {
public:
    explicit World(auto& entities) : entities_{entities} {};
    void update(float dt);
private:
    std::unordered_map<uint32_t, std::unique_ptr<shared::Entity>>& entities_;
};

}

#endif // SERVER_WORLD_H
