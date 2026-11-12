#ifndef SHARED_MAP_H
#define SHARED_MAP_H

#include "shared/Messages.h"
#include "shared/Protocol.h"
#include <cstdint>
#include <cstddef>
#include <span>
#include <vector>
#include <filesystem>
#include <expected>

namespace shared {

class BitGrid {
public:
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

private:
    uint32_t w_;
    uint32_t h_;
    uint32_t wordsPerRow_;
    std::vector<uint64_t> bits_;
};

class Map {
public:
    enum class MapError {
        FileNotFound,
        SizeMismatch,
        InvalidJson
    };

    static std::expected<Map, MapError> fromFile(std::filesystem::path &filepath);
    static std::expected<Map, MapError> fromJson(std::string_view json);
    static std::expected<Map, MapError> fromNetwork(shared::MapMessage map);
private:
    size_t width_ = 0;
    size_t height_ = 0;
    std::vector<uint16_t> layer_;
    BitGrid collisions_;
};

}

#endif
