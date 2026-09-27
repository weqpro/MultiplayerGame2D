#ifndef SERIALIZATION__H_
#define SERIALIZATION__H_

#include <span>

namespace shared {

class Writer {
public:
    constexpr explicit Writer(std::span<std::byte> buf) noexcept;

    template <typename T>
    constexpr void write(T value) noexcept;
    [[nodiscard]] constexpr bool ok() const noexcept;
    [[nodiscard]] constexpr std::size_t written() const noexcept;
};

class Reader {
public:
    constexpr explicit Reader(std::span<std::byte> buf) noexcept;

    template <typename T>
    [[nodiscard]] constexpr T read() noexcept;
    [[nodiscard]] constexpr bool ok() const noexcept;
    [[nodiscard]] constexpr std::size_t written() const noexcept;
};

}

#endif
