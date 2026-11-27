#ifndef SERVER_OCCUPANCY_MAP_H
#define SERVER_OCCUPANCY_MAP_H

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace server {

class OccupancyMap {
public:
    OccupancyMap() = default;
    OccupancyMap(size_t w, size_t h) : w_(w), h_(h), counts_(w * h, 0) {}

    void clear() { std::fill(counts_.begin(), counts_.end(), 0); }

    void mark(int x, int y) {
        if (x >= 0 && y >= 0 && size_t(x) < w_ && size_t(y) < h_)
            ++counts_[size_t(y) * w_ + size_t(x)];
    }

    uint8_t count(int x, int y) const {
        if (x < 0 || y < 0 || size_t(x) >= w_ || size_t(y) >= h_) return 0;
        return counts_[size_t(y) * w_ + size_t(x)];
    }

    size_t width() const { return w_; }
    size_t height() const { return h_; }

private:
    size_t w_ = 0, h_ = 0;
    std::vector<uint8_t> counts_;
};

}

#endif
