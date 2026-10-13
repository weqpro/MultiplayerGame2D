#ifndef SHARED_PROTOCOL_H
#define SHARED_PROTOCOL_H

#include <tuple>
#include <vector>

#include "shared/Messages.h"
#include "shared/WebContext.h"
#include "shared/types/Body.h"
#include "shared/types/ExternalFields.h"
#include "shared/types/Ids.h"
#include "shared/types/Intent.h"
#include "shared/types/SpawnInfo.h"


namespace shared {

struct IntentMessage {
    static constexpr MessageType TYPE = MessageType::INTENT;
    static constexpr network::Channel CHANNEL = network::UNRELIABLE;

    InputSeq seq{};
    Intent intent{};

    static constexpr auto fields(auto& self) { return std::tie(self.seq, self.intent); }
};

struct PossessMessage {
    static constexpr MessageType TYPE = MessageType::POSSESS;
    static constexpr network::Channel CHANNEL = network::RELIABLE;

    EntityId pawn{NULL_ENTITY};

    static constexpr auto fields(auto& self) { return std::tie(self.pawn); }
};

struct SpawnMessage {
    static constexpr MessageType TYPE = MessageType::SPAWN;
    static constexpr network::Channel CHANNEL = network::RELIABLE;

    std::vector<SpawnInfo> entities;

    static constexpr auto fields(auto& self) { return std::tie(self.entities); }
};

struct DespawnMessage {
    static constexpr MessageType TYPE = MessageType::DESPAWN;
    static constexpr network::Channel CHANNEL = network::RELIABLE;

    std::vector<EntityId> ids;

    static constexpr auto fields(auto& self) { return std::tie(self.ids); }
};

struct EntityState {
    EntityId id{};
    Body body{};

    static constexpr auto fields(auto& self) { return std::tie(self.id, self.body); }
};

struct SnapshotMessage {
    static constexpr MessageType TYPE = MessageType::SNAPSHOT;
    static constexpr network::Channel CHANNEL = network::UNRELIABLE;

    Tick tick{};
    std::vector<EntityState> entities;

    static constexpr auto fields(auto& self) { return std::tie(self.tick, self.entities); }
};

}

#endif
