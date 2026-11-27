#include "shared/entities/EntityFactory.h"

#include "shared/entities/NPC.h"
#include "shared/entities/Player.h"

namespace shared {

std::unique_ptr<Entity> makeEntity(const SpawnInfo& info) { // TODO: add attributes
    if (info.id == NULL_ENTITY) {
        return nullptr;
    }
    switch (info.kind) {
        case EntityKind::PLAYER:
            return std::make_unique<Player>(info.id, info.body, info.look, info.name);
        case EntityKind::NPC:
            return std::make_unique<NPC>(info.id, info.body, info.look);
        case EntityKind::COUNT:
            break;
    }
    return nullptr;
}

}
