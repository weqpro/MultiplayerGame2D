#ifndef SHARED_ENTITIES_ENTITY_H
#define SHARED_ENTITIES_ENTITY_H

#include <string_view>

#include "shared/types/Appearance.h"
#include "shared/types/Body.h"
#include "shared/types/EntityKind.h"
#include "shared/types/Ids.h"
#include "shared/types/SpawnInfo.h"

namespace shared {

class Entity {
public:
    virtual ~Entity() = default;

    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;

    [[nodiscard]] EntityId id() const noexcept { return id_; }
    [[nodiscard]] virtual EntityKind kind() const noexcept = 0;

    [[nodiscard]] const Body& body() const noexcept { return body_; }
    void setBody(const Body& body) noexcept { body_ = body; }

    [[nodiscard]] const Appearance& appearance() const noexcept { return look_; }
    [[nodiscard]] virtual std::string_view label() const noexcept { return {}; }

    [[nodiscard]] virtual SpawnInfo describe() const;

protected:
    Entity(EntityId id, const Body& body, const Appearance& look) : id_{id}, body_{body}, look_{look} {}

private:
    EntityId id_;
    Body body_;
    Appearance look_;
};

}

#endif
