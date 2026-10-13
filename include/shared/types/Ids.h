#ifndef SHARED_TYPES_IDS_H
#define SHARED_TYPES_IDS_H

#include <cstdint>

namespace shared {

using EntityId = std::uint32_t;
using Tick = std::uint32_t;
using InputSeq = std::uint32_t;

inline constexpr EntityId NULL_ENTITY = 0;

}

#endif
