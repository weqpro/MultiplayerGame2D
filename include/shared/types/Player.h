#ifndef SHARED_TYPES_PLAYER_H
#define SHARED_TYPES_PLAYER_H

#include <cstdint>
#include <tuple>
#include "shared/Messages.h"
#include "shared/types/Body.h"

namespace shared {

struct PlayerDTO {
    std::uint32_t id{};
    Body body;
    
    static constexpr auto fields(auto& self) { return std::tie(self.id, self.body); }
};

struct PlayersMessage {
    static constexpr MessageType TYPE = MessageType::PLAYERS;
 
    std::vector<Player> players;
 
    static constexpr auto fields(auto& self) { return std::tie(self.players); }
};

}

#endif
