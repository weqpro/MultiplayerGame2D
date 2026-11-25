#ifndef SHARED_PROTOCOL_H
#define SHARED_PROTOCOL_H

#include <tuple>
#include <variant>
#include <vector>

#include "shared/Messages.h"
#include "shared/WebContext.h"
#include "shared/types/Action.h"
#include "shared/types/Body.h"
#include "shared/types/ExternalFields.h"
#include "shared/types/Ids.h"
#include "shared/types/Intent.h"
#include "shared/types/SpawnInfo.h"


namespace shared {

struct ControlsMessage {
    static constexpr network::Channel CHANNEL = network::UNRELIABLE;

    InputSeq seq{};
    Controls controls{};

    static constexpr auto fields(auto& self) { return std::tie(self.seq, self.controls); }
};

template <typename A>
struct ActionMessage {
    static constexpr network::Channel CHANNEL = network::RELIABLE;

    A action{};

    static constexpr auto fields(auto& self) { return std::tie(self.action); }
};

struct PossessMessage {
    static constexpr network::Channel CHANNEL = network::RELIABLE;

    EntityId pawn{NULL_ENTITY};

    static constexpr auto fields(auto& self) { return std::tie(self.pawn); }
};

struct SpawnMessage {
    static constexpr network::Channel CHANNEL = network::RELIABLE;

    std::vector<SpawnInfo> entities;

    static constexpr auto fields(auto& self) { return std::tie(self.entities); }
};

struct DespawnMessage {
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
    static constexpr network::Channel CHANNEL = network::UNRELIABLE;

    Tick tick{};
    std::vector<EntityState> entities;

    static constexpr auto fields(auto& self) { return std::tie(self.tick, self.entities); }
};

struct MapInfo {
    size_t width = 0;
    size_t height = 0;
    std::vector<uint16_t> layer;

    static constexpr auto fields(auto& self) { return std::tie(self.width, self.height, self.layer); }
};

struct MapMessage {
    static constexpr network::Channel CHANNEL = network::RELIABLE;

    MapInfo map {};
    static constexpr auto fields(auto& self) { return std::tie(self.map); }
};
 
template <typename V>
struct ClientMessagesOf;

template <typename... As>
struct ClientMessagesOf<std::variant<As...>> {
    using type = MessageList<ControlsMessage, ActionMessage<As>...>;
};

// Wire id = position in the list. Client and server are always built from the same source.
// ControlsMessage = 0, then one ActionMessage<A> per Action alternative, in variant order.
using ClientMessages = ClientMessagesOf<Action>::type;
using ServerMessages = MessageList<PossessMessage, SpawnMessage, DespawnMessage, SnapshotMessage>;


}

#endif
