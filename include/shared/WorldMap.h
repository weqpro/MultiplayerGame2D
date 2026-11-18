#ifndef SHARED_MAP_H
#define SHARED_MAP_H

#include "shared/Protocol.h"
#include <cstdint>
#include <cstddef>
#include <span>
#include <vector>
#include <filesystem>
#include <expected>
#include <utility>

namespace shared {

class BitGrid {
public:
    BitGrid() = default;
    BitGrid(BitGrid&& o) noexcept
    : w_(std::exchange(o.w_, 0)), h_(std::exchange(o.h_, 0)),
      wordsPerRow_(std::exchange(o.wordsPerRow_, 0)), bits_(std::move(o.bits_)) {}
    BitGrid(const BitGrid& other) = delete;
    BitGrid& operator=(BitGrid&& o) noexcept;
    BitGrid& operator=(const BitGrid& o) = delete;

    BitGrid(uint32_t w, uint32_t h)
        : w_(w), h_(h), wordsPerRow_((w + 63) / 64),
        bits_(size_t(wordsPerRow_) * h) {}

    bool get(uint32_t x, uint32_t y) const {
        return (bits_[size_t(y) * wordsPerRow_ + (x >> 6)] >> (x & 63)) & 1;
    }
    void set(uint32_t x, uint32_t y) {
        bits_[size_t(y) * wordsPerRow_ + (x >> 6)] |= uint64_t(1) << (x & 63);
    }
    std::span<const uint64_t> row(uint32_t y) const {
        return {&bits_[size_t(y) * wordsPerRow_], wordsPerRow_};
    }

    BitGrid(size_t w, size_t h, std::vector<uint64_t>&& bits) : w_(w), h_(h), wordsPerRow_((w + 63) / 64), bits_(std::move(bits)) {}

    uint32_t width()  const { return w_; }
    uint32_t height() const { return h_; }
private:
    uint32_t w_ = 0;
    uint32_t h_ = 0;
    uint32_t wordsPerRow_ = 0;
    std::vector<uint64_t> bits_ {};
};

class WorldMap {
public:
    enum class MapError {
        FileNotFound,
        SizeMismatch,
        InvalidJson
    };

    WorldMap() = default;
    WorldMap(size_t width, size_t height, std::vector<uint16_t> layer, BitGrid collisions_) : 
        width_(width), height_(height), layer_(std::move(layer)), collisions_(std::move(collisions_)){}
    WorldMap(WorldMap&& o) noexcept
        : width_(std::exchange(o.width_, 0)),
          height_(std::exchange(o.height_, 0)),
          layer_(std::exchange(o.layer_, {})),
          collisions_(std::move(o.collisions_)) {}
    WorldMap(const WorldMap& o) = delete;
    WorldMap& operator=(WorldMap&& other) noexcept;
    WorldMap& operator=(const WorldMap& other) = delete;

    static std::expected<WorldMap, MapError> fromFile(const std::filesystem::path &filepath);
    static std::expected<WorldMap, MapError> fromJson(std::string_view json);
    static std::expected<WorldMap, MapError> fromNetwork(shared::MapMessage map);
    size_t width()  const { return width_; }
    size_t height() const { return height_; }
    const std::vector<uint16_t> layer() const {return layer_; }

    uint16_t tile(size_t x, size_t y) const { return layer_[y * width_ + x]; }

    bool solid(int x, int y) const {
        if (x < 0 || y < 0 || size_t(x) >= width_ || size_t(y) >= height_) return true;
        return collisions_.get(uint32_t(x), uint32_t(y));
    }

private:
    size_t width_ = 0;
    size_t height_ = 0;
    std::vector<uint16_t> layer_ {};
    BitGrid collisions_ {};
};

}

#endif
