#ifndef SHARED_TYPES_UNITS_H
#define SHARED_TYPES_UNITS_H

namespace shared {

using HealthPoint = float;

inline namespace literals {

consteval HealthPoint operator""_hp(unsigned long long value) { return static_cast<HealthPoint>(value); }
consteval HealthPoint operator""_hp(long double value) { return static_cast<HealthPoint>(value); }

}

}

#endif
