#ifndef SERVER_WORLD_H
#define SERVER_WORLD_H

#include <cassert>
#include <concepts>
#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "shared/WorldMap.h"
#include "shared/contracts/IController.h"
#include "shared/entities/Agent.h"
#include "shared/entities/EntityRegistry.h"
#include "shared/types/Ids.h"
#include "shared/controllers/CameraController.h"

namespace server {

class World {
public:
    struct Events {
        std::vector<shared::EntityId> spawned;
        std::vector<shared::EntityId> despawned;
    };

    template <std::derived_from<shared::Entity> T, typename... Args>
    T& spawn(Args&&... args) {
        auto entity = std::make_unique<T>(nextId_++, std::forward<Args>(args)...);
        T& ref = *entity;
        registry_.insert(std::move(entity));
        events_.spawned.push_back(ref.id());
        return ref;
    }

    // Also drops the entity's controller, if it has one.
    void despawn(shared::EntityId id);

    template <std::derived_from<shared::IController> C, typename... Args>
    C& possess(shared::Agent& pawn, Args&&... args) {
        assert(registry_.find(pawn.id()) == &pawn);
        auto controller = std::make_unique<C>(std::forward<Args>(args)...);
        C& ref = *controller;
        possessions_[pawn.id()] = Possession{&pawn, std::move(controller)};
        return ref;
    }

    void release(shared::EntityId pawn);

    template <std::derived_from<shared::IController> C>
    [[nodiscard]] C* controllerOf(shared::EntityId pawn) noexcept {
        const auto it = possessions_.find(pawn);
        return it == possessions_.end() ? nullptr : dynamic_cast<C*>(it->second.controller.get());
    }

    void update(float dt);

    [[nodiscard]] Events drainEvents();

    [[nodiscard]] const shared::EntityRegistry& entities() const noexcept { return registry_; }
    [[nodiscard]] const shared::WorldMap& worldMap() const noexcept { return worldMap_; }
    [[nodiscard]] CameraWrapper& camera() noexcept { return cam_; }
    [[nodiscard]] const CameraWrapper& camera() const noexcept { return cam_; }
    void setMap(const std::filesystem::path& mapPath = "assets/test_map.json");

private:
    struct Possession {
        shared::Agent* pawn;
        std::unique_ptr<shared::IController> controller;
    };

    shared::EntityRegistry registry_;
    std::unordered_map<shared::EntityId, Possession> possessions_;
    shared::EntityId nextId_{1};
    Events events_;
    shared::WorldMap worldMap_{};
    CameraWrapper cam_;
};

}

#endif // SERVER_WORLD_H
