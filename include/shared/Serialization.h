#ifndef SERIALIZATION__H_
#define SERIALIZATION__H_

#include <bit>
#include <cstddef>
#include <span>
#include <limits>
#include <concepts>
#include <cstdint>
#include <ranges>
#include <type_traits>

namespace shared {

static_assert(std::numeric_limits<float>::is_iec559,  "wire format requires IEEE-754 binary32");
static_assert(std::numeric_limits<double>::is_iec559, "wire format requires IEEE-754 binary64");

template <typename T>
concept Serializable =
    (std::integral<T> || std::floating_point<T> || std::is_enum_v<T>) &&
    !std::same_as<std::remove_cv_t<T>, bool> &&
    (sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8);

using RangeSizeT = std::uint16_t;

template <typename R>
concept SerializableRange =
    std::ranges::contiguous_range<R> &&
    std::ranges::sized_range<R> &&
    Serializable<std::ranges::range_value_t<R>>;

template <typename R>
concept DeserializableRange =
    SerializableRange<R> &&
    std::default_initializable<R> &&
    requires(R& c, std::size_t n, std::ranges::range_value_t<R> v) {
        c.reserve(n);
        c.push_back(v);
    };

namespace detail {

template <std::size_t N> struct uint_of;
template <> struct uint_of<1> { using type = std::uint8_t;  };
template <> struct uint_of<2> { using type = std::uint16_t; };
template <> struct uint_of<4> { using type = std::uint32_t; };
template <> struct uint_of<8> { using type = std::uint64_t; };

} // namespace detail

template<Serializable T>
using uint_for = typename detail::uint_of<sizeof(T)>::type;

class Writer {
public:
    constexpr explicit Writer(std::span<std::byte> buf) noexcept : buf_{buf} {}

    template <Serializable T>
    constexpr void write(T value) noexcept {
        if (sizeof(T) > buf_.size() - pos_) { failed_ = true; }
        if (failed_) { return; }

        const auto bits = std::bit_cast<uint_for<T>>(value);

        for (size_t i = 0; i < sizeof(T); ++i) {
            buf_[pos_ + i] = static_cast<std::byte>(bits >> (8 * (sizeof(T) - 1 - i))); // in network order
        }

        pos_ += sizeof(T);
    }

    template <SerializableRange R>
    constexpr void write(R&& range) noexcept {
        std::span values{range};

        constexpr auto elementSize = sizeof(std::ranges::range_value_t<R>);

        if (values.size() > std::numeric_limits<RangeSizeT>::max() ||
            sizeof(RangeSizeT) + values.size() * elementSize > buf_.size() - pos_) {
            failed_ = true;
            return;
        }

        auto size = static_cast<RangeSizeT>(values.size());
        write(size);
        for (const auto& v : values) {
            write(v);
        }
    }

    constexpr void fail() noexcept { failed_ = true; }
    [[nodiscard]] constexpr bool ok() const noexcept { return !failed_; }
    [[nodiscard]] constexpr std::size_t written() const noexcept { return pos_; }

private:
    std::span<std::byte> buf_;
    size_t pos_ = 0;
    bool failed_ = false;
};

class Reader {
public:
    constexpr explicit Reader(std::span<const std::byte> buf) noexcept : buf_{buf} {}

    template <Serializable T>
    [[nodiscard]] constexpr T read() noexcept {
        using U = uint_for<T>;

        if (sizeof(T) > buf_.size() - pos_ ) { failed_ = true; }
        if (failed_) { return T{}; }

        U bits = 0;

        for (size_t i = 0; i < sizeof(T); ++i) {
            bits = static_cast<U>((bits << 8) | std::to_integer<U>(buf_[pos_ + i]));
        }

        pos_ += sizeof(T);
        return std::bit_cast<T>(bits);
    }

    template <DeserializableRange R>
    [[nodiscard]] constexpr R read() {
        using T = std::ranges::range_value_t<R>;

        const auto n = read<RangeSizeT>();
        if (failed_ || n * sizeof(T) > remaining()) {
            failed_ = true;
            return R{};
        }

        R out;
        out.reserve(n);
        for (std::size_t i = 0; i < n; ++i) {
            out.push_back(read<T>());
        }
        return out;
    }

    constexpr void fail() noexcept { failed_ = true; }
    [[nodiscard]] constexpr bool ok() const noexcept { return !failed_; }
    [[nodiscard]] constexpr size_t remaining() const noexcept { return buf_.size() - pos_; }

private:
    std::span<const std::byte> buf_;
    size_t pos_ = 0;
    bool failed_ = false;
};

}

#endif
