#ifndef SHARED_TYPES_UNITS_H
#define SHARED_TYPES_UNITS_H

namespace shared {

using health_point = float;

inline namespace literals {

consteval health_point operator""_hp(unsigned long long value) { return static_cast<health_point>(value); }
consteval health_point operator""_hp(long double value) { return static_cast<health_point>(value); }

}

}

#endif
