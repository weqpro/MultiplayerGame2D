#ifndef SHARED_MESSAGES_H
#define SHARED_MESSAGES_H

#include <algorithm>
#include <array>
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
#include <utility>

#include <raylib.h>

#include "shared/Serialization.h"

namespace shared {

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
    std::default_initializable<T>;

using WireId = std::uint16_t;

enum class DispatchResult {
    OK,
    UNKNOWN_ID,
    MALFORMED
};

template <typename T, typename... Ts>
inline constexpr std::size_t count_of = (std::size_t{std::same_as<T, Ts>} + ... + 0);

template <typename... Ts>
inline constexpr bool distinct = ((count_of<Ts, Ts...> == 1) && ...);

template <Message M, typename Handler>
DispatchResult decodeAndHandle(Reader& r, Handler& handler) {
    M message{};
    decode(r, message);
    if (!r.ok() || r.remaining() != 0) {
        return DispatchResult::MALFORMED;
    }
    handler(std::as_const(message));
    return DispatchResult::OK;
}

// Wire id of a message = its position in the list.
template <Message... Ms>
struct MessageList {
    static_assert(sizeof...(Ms) > 0);
    static_assert(sizeof...(Ms) <= std::size_t{std::numeric_limits<WireId>::max()} + 1, "too many messages for WireId");
    static_assert(distinct<Ms...>, "a message appears twice in the list");

    template <typename M>
    static constexpr bool contains = (std::same_as<M, Ms> || ...);

    template <typename M>
        requires contains<M>
    static constexpr WireId idOf = [] {
        constexpr std::array matches{std::same_as<M, Ms>...};
        return static_cast<WireId>(std::ranges::find(matches, true) - matches.begin());
    }();

    // Expands to `(id == 0 && decode<M0>) || (id == 1 && decode<M1>) || ...`, lowered like a switch.
    template <typename Handler>
    static DispatchResult dispatch(Reader& r, WireId id, Handler& handler) {
        auto result = DispatchResult::UNKNOWN_ID;
        [&]<std::size_t... I>(std::index_sequence<I...>) {
            (void)((id == I && (result = decodeAndHandle<Ms>(r, handler), true)) || ...);
        }(std::index_sequence_for<Ms...>{});
        return result;
    }
};

template <typename List, Message M>
    requires List::template contains<M>
[[nodiscard]] std::optional<std::vector<std::byte>> encodeMessage(const M& message) {
    std::vector<std::byte> buf(sizeof(WireId) + encoded_size(message));
    Writer w{buf};
    w.write(List::template idOf<M>);
    encode(w, message);
    if (!w.ok()) {
        return std::nullopt;
    }
    return buf;
}

template <typename List, typename Handler>
DispatchResult dispatch(std::span<const std::byte> bytes, Handler&& handler) {
    Reader r{bytes};
    const auto id = r.read<WireId>();
    if (!r.ok()) {
        return DispatchResult::MALFORMED;
    }
    return List::dispatch(r, id, handler);
}

}

#endif // SHARED_MESSAGES_H
