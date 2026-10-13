#ifndef SHARED_ENTITIES_ENTITY_REGISTRY_H
#define SHARED_ENTITIES_ENTITY_REGISTRY_H

#include <concepts>
#include <cstddef>
#include <memory>
#include <ranges>
#include <unordered_map>

#include "shared/entities/Entity.h"
#include "shared/types/Ids.h"

namespace shared {

class EntityRegistry {
public:
    // Replaces the entity with the same id, if there is one.
    Entity& insert(std::unique_ptr<Entity> entity);

    std::unique_ptr<Entity> erase(EntityId id);

    [[nodiscard]] Entity* find(EntityId id) noexcept;
    [[nodiscard]] const Entity* find(EntityId id) const noexcept;

    template <std::derived_from<Entity> T>
    [[nodiscard]] T* find(EntityId id) noexcept { return dynamic_cast<T*>(find(id)); }

    template <std::derived_from<Entity> T>
    [[nodiscard]] const T* find(EntityId id) const noexcept { return dynamic_cast<const T*>(find(id)); }

    [[nodiscard]] auto all() const {
        return entities_ | std::views::values
             | std::views::transform([](const std::unique_ptr<Entity>& e) -> const Entity& { return *e; });
    }

    [[nodiscard]] std::size_t size() const noexcept { return entities_.size(); }

private:
    std::unordered_map<EntityId, std::unique_ptr<Entity>> entities_;
};

}

#endif
