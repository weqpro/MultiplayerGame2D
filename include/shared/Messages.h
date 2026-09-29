#ifndef MESSAGES__H_
#define MESSAGES__H_

#include <concepts>
#include <type_traits>
#include <cstddef>
#include <array>
#include <cstdint>
#include <iostream>

#include <raylib.h>

#include "shared/Serialization.h"

namespace shared {

enum class MessageType : uint16_t {
    INTENT,
    BODIES,
    COUNT
};

template <typename T>
concept SerializableMessage = requires(const T t, Writer& writer) {
    { t.serialize(writer) } -> std::same_as<void>;
    typename std::integral_constant<std::size_t, T::SIZE>;
};

template<SerializableMessage T>
struct Packet {
    MessageType type;
    T& message;

    static constexpr size_t SIZE = T::SIZE + sizeof(std::underlying_type_t<MessageType>);

    std::array<const std::byte, SIZE> serialize() const {
        std::array<const std::byte, SIZE> data;
        Writer writer{data};

        uint16_t messageTypeValue = static_cast<std::underlying_type_t<MessageType>>(type);
        writer.write(messageTypeValue);
        message.serialize(writer);

        if (!writer.ok()) {
            std::cout << "Packet serialization not ok\n";
        }
        if (writer.written() != SIZE) {
            std::cout << "Packet serialization did not write the whole buffer\n";
        }

        return data;
    }
};

class IntentMessage {
public:
    static constexpr size_t SIZE = sizeof(float) * 2;
    Vector2 intent;

    void serialize(Writer& writer) const;
};

}

#endif // !MESSAGES__H_
