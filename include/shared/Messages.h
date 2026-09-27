#ifndef MESSAGES__H_
#define MESSAGES__H_

#include <concepts>
#include <string_view>

namespace shared {

template <typename T>
concept Serializable = requires(T t) {
    { t.serialize() } -> std::same_as<std::string_view>;
};

template<Serializable T>
struct Message {
    T& data;
};

}

#endif // !MESSAGES__H_
