#include "shared/entities/EntityRegistry.h"

#include <cassert>
#include <utility>

namespace shared {

Entity& EntityRegistry::insert(std::unique_ptr<Entity> entity) {
    assert(entity && entity->id() != NULL_ENTITY);
    auto& slot = entities_[entity->id()];
    slot = std::move(entity);
    return *slot;
}

std::unique_ptr<Entity> EntityRegistry::erase(EntityId id) {
    auto node = entities_.extract(id);
    return node ? std::move(node.mapped()) : nullptr;
}

Entity* EntityRegistry::find(EntityId id) noexcept {
    const auto it = entities_.find(id);
    return it == entities_.end() ? nullptr : it->second.get();
}

const Entity* EntityRegistry::find(EntityId id) const noexcept {
    const auto it = entities_.find(id);
    return it == entities_.end() ? nullptr : it->second.get();
}

}
