#ifndef SHARED_CONTAINERS_FIXED_VECTOR_H
#define SHARED_CONTAINERS_FIXED_VECTOR_H

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace shared {

// std::array plus a count: heap-free, trivially copyable. Stand-in for C++26 std::inplace_vector.
template <typename T, std::size_t N>
    requires std::is_trivially_copyable_v<T> && std::default_initializable<T>
class FixedVector {
public:
    static_assert(N > 0 && N <= std::numeric_limits<std::uint8_t>::max());

    // Returns false and drops the value when full.
    constexpr bool push_back(const T& value) noexcept {
        if (size_ == N) {
            return false;
        }
        items_[size_++] = value;
        return true;
    }

    constexpr void clear() noexcept { size_ = 0; }

    // Removes the first n elements, keeping the order of the rest.
    constexpr void dropFront(std::size_t n) noexcept {
        n = std::min<std::size_t>(n, size_);
        std::copy(items_.begin() + n, items_.begin() + size_, items_.begin());
        size_ = static_cast<std::uint8_t>(size_ - n);
    }

    [[nodiscard]] constexpr std::size_t size() const noexcept { return size_; }
    [[nodiscard]] constexpr bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] static constexpr std::size_t capacity() noexcept { return N; }

    [[nodiscard]] constexpr const T* begin() const noexcept { return items_.data(); }
    [[nodiscard]] constexpr const T* end() const noexcept { return items_.data() + size_; }

private:
    std::array<T, N> items_{};
    std::uint8_t size_{0};
};

}

#endif
