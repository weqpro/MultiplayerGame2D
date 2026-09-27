#ifndef MESSAGES__H_
#define MESSAGES__H_

#include <concepts>
#include <cstddef>
#include <span>

namespace shared {

template <typename T>
concept Serializable = requires(T t) {
    { t.serialize() } -> std::same_as<std::span<const std::byte>>;
};

template<Serializable T>
struct Message {
    T& data;
};

}

#endif // !MESSAGES__H_
