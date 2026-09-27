#ifndef MESSAGES__H_
#define MESSAGES__H_

#include <concepts>
#include <cstddef>
#include <span>
#include <cstdint>


namespace shared {

enum class MessageType : uint8_t {
    INTENT,
    PLAYER,
    COUNT
};

template <typename T>
concept Serializable = requires(T t) {
    { t.serialize() } -> std::same_as<std::span<const std::byte>>;
};

template<Serializable T>
struct Packet {
    MessageType type;
    T& message;
};

class IntentMessage {
public:
    static constexpr size_t SIZE = sizeof(float) * 2;
    float x;
    float y;
    uint8_t controllerId;

    std::span<const std::byte> serialize();
private:
    std::array<std::byte, SIZE> data;
};

}

#endif // !MESSAGES__H_
