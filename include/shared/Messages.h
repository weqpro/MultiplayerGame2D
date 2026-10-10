#ifndef SHARED_MESSAGES_H
#define SHARED_MESSAGES_H

#include <concepts>
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <tuple>
#include <optional>
#include <span>
#include <vector>
#include <limits>
#include <ranges>

#include <raylib.h>

#include "shared/Serialization.h"

namespace shared {

enum class MessageType : uint16_t {
    INTENT,
    PLAYERS,
    COUNT
};

template <typename T>
struct ExternalFields {};

 
template <typename T>
concept HasMemberFields = requires(T& t, const T& ct) {
    T::fields(t);
    T::fields(ct);
};
 
template <typename T>
concept HasExternalFields = requires(T& t, const T& ct) {
    ExternalFields<T>::get(t);
    ExternalFields<T>::get(ct);
};
 
template <typename T>
concept Structured = HasMemberFields<T> || HasExternalFields<T>;
 
template <typename T>
    requires Structured<std::remove_const_t<T>>
constexpr auto fields_of(T& value) {
    using U = std::remove_const_t<T>;
    if constexpr (HasMemberFields<U>) {
        return U::fields(value);
    } else {
        return ExternalFields<U>::get(value);
    }
}

template <typename T>
concept StructuredRange =
    std::ranges::sized_range<const T&> &&
    Structured<std::ranges::range_value_t<T>>;
 
template <typename T>
concept DecodableStructuredRange =
    StructuredRange<T> &&
    std::default_initializable<T> &&
    requires(T& c, std::size_t n, std::ranges::range_value_t<T> e) {
        c.reserve(n);
        c.push_back(std::move(e));
    };

template <typename T>
constexpr void encode(Writer& w, const T& value) {
    if constexpr (Serializable<T> || SerializableRange<const T&>) {
        w.write(value);
    } else if constexpr (Structured<T>) {
        std::apply([&w](const auto&... f) { (encode(w, f), ...); }, fields_of(value));
    } else if constexpr (StructuredRange<T>) {
        const auto n = std::ranges::size(value);
        if (n > std::numeric_limits<RangeSizeT>::max()) {
            w.fail();
            return;
        }
        w.write(static_cast<RangeSizeT>(n));
        for (const auto& e : value) {
            encode(w, e);
        }
    } else {
        static_assert(sizeof(T) == 0, "not encodable: add a fields() list or an ExternalFields specialization");
    }
}

template <typename T>
constexpr void decode(Reader& r, T& value) {
    if constexpr (Serializable<T> || DeserializableRange<T>) {
        value = r.read<T>();
    } else if constexpr (Structured<T>) {
        std::apply([&r](auto&... f) { (decode(r, f), ...); }, fields_of(value));
    } else if constexpr (DecodableStructuredRange<T>) {
        const std::size_t n = r.read<RangeSizeT>();
        if (!r.ok() || n > r.remaining()) {
            r.fail();
            return;
        }
        value = T{};
        value.reserve(n);
        for (std::size_t i = 0; i < n && r.ok(); ++i) {
            std::ranges::range_value_t<T> e{};
            decode(r, e);
            value.push_back(std::move(e));
        }
    } else {
        static_assert(sizeof(T) == 0, "not decodable: add a fields() list or an ExternalFields specialization");
    }
}

template <typename T>
[[nodiscard]] constexpr std::size_t encoded_size(const T& value) {
    if constexpr (Serializable<T>) {
        return sizeof(T);
    } else if constexpr (SerializableRange<const T&>) {
        return sizeof(RangeSizeT) + std::ranges::size(value) * sizeof(std::ranges::range_value_t<T>);
    } else if constexpr (Structured<T>) {
        return std::apply([](const auto&... f) { return (std::size_t{0} + ... + encoded_size(f)); },
                          fields_of(value));
    } else if constexpr (StructuredRange<T>) {
        std::size_t size = sizeof(RangeSizeT);
        for (const auto& e : value) {
            size += encoded_size(e);
        }
        return size;
    } else {
        static_assert(sizeof(T) == 0, "not encodable: add a fields() list or an ExternalFields specialization");
    }
}

template <typename T>
concept Message =
    Structured<T> &&
    std::default_initializable<T> &&
    std::same_as<std::remove_cv_t<decltype(T::TYPE)>, MessageType>;

[[nodiscard]] constexpr std::optional<MessageType> peek_type(std::span<const std::byte> bytes) noexcept {
    using U = std::underlying_type_t<MessageType>;
    Reader r{bytes};
    const auto type = r.read<MessageType>();
    if (!r.ok() || static_cast<U>(type) >= static_cast<U>(MessageType::COUNT)) {
        return std::nullopt;
    }
    return type;
}

template <Message M>
struct Packet {
    static constexpr MessageType TYPE = M::TYPE;
    static constexpr std::size_t HEADER_SIZE = sizeof(MessageType);
 
    M message;
 
    [[nodiscard]] constexpr std::size_t size() const {
        return HEADER_SIZE + encoded_size(message);
    }
 
    [[nodiscard]] constexpr std::size_t serialize(std::span<std::byte> out) const {
        Writer w{out};
        w.write(TYPE);
        encode(w, message);
        return w.ok() ? w.written() : 0;
    }
 
    [[nodiscard]] std::optional<std::vector<std::byte>> serialize() const {
        std::vector<std::byte> buf(size());
        if (serialize(std::span{buf}) == 0) {
            return std::nullopt;
        }
        return buf;
    }
 
    [[nodiscard]] static constexpr std::optional<Packet> deserialize(std::span<const std::byte> bytes) {
        Reader r{bytes};
        const auto type = r.read<MessageType>();
        if (!r.ok() || type != TYPE) {
            return std::nullopt;
        }
        Packet packet{};
        decode(r, packet.message);
        if (!r.ok() || r.remaining() != 0) {
            return std::nullopt;
        }
        return packet;
    }
};

template <Message M>
Packet(M) -> Packet<M>; // клодік сказав, шо мало б без цього всюди воркати, але clang може і не справитись
// clang 17+, GCC 11+ and MSVC 19.27+

}

#endif // SHARED_MESSAGES_H
