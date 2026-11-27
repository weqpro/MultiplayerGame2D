#include "server/FlowField.h"

#include <limits>
#include <queue>
#include <utility>

#include <raymath.h>

namespace server {

namespace {

constexpr float INF = std::numeric_limits<float>::infinity();
constexpr float SQRT2 = 1.41421356f;

constexpr int DX[] = { 1, -1, 0,  0, 1, -1,  1, -1};
constexpr int DY[] = { 0,  0, 1, -1, 1,  1, -1, -1};
constexpr float MOVE_COST[] = {1, 1, 1, 1, SQRT2, SQRT2, SQRT2, SQRT2};

}

FlowField::FlowField(size_t w, size_t h)
    : w_(w), h_(h),
      costField_(w * h, INF),
      dirField_(w * h, Vector2{0, 0}) {}

void FlowField::compute(const shared::MapView& map, const OccupancyMap& occupancy,
                         int goalX, int goalY, float occupancyWeight) {
    goalX_ = goalX;
    goalY_ = goalY;

    const size_t total = w_ * h_;
    std::fill(costField_.begin(), costField_.end(), INF);
    std::fill(dirField_.begin(), dirField_.end(), Vector2{0, 0});

    if (goalX < 0 || goalY < 0 || size_t(goalX) >= w_ || size_t(goalY) >= h_) return;
    if (map.isSolid(goalX, goalY)) return;

    using Entry = std::pair<float, int>;
    std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> open;

    const int goalIdx = goalY * int(w_) + goalX;
    costField_[goalIdx] = 0.0f;
    open.push({0.0f, goalIdx});

    while (!open.empty()) {
        auto [curCost, curIdx] = open.top();
        open.pop();

        if (curCost > costField_[curIdx]) continue;

        const int cx = curIdx % int(w_);
        const int cy = curIdx / int(w_);

        for (int d = 0; d < 8; ++d) {
            const int nx = cx + DX[d];
            const int ny = cy + DY[d];

            if (nx < 0 || ny < 0 || size_t(nx) >= w_ || size_t(ny) >= h_) continue;
            if (map.isSolid(nx, ny)) continue;

            if (d >= 4) {
                if (map.isSolid(cx + DX[d], cy) || map.isSolid(cx, cy + DY[d])) continue;
            }

            const float penalty = float(occupancy.count(nx, ny)) * occupancyWeight;
            const float newCost = curCost + MOVE_COST[d] + penalty;
            const int nIdx = ny * int(w_) + nx;

            if (newCost < costField_[nIdx]) {
                costField_[nIdx] = newCost;
                open.push({newCost, nIdx});
            }
        }
    }

    for (size_t y = 0; y < h_; ++y) {
        for (size_t x = 0; x < w_; ++x) {
            const size_t idx = y * w_ + x;
            if (costField_[idx] >= INF) continue;

            float bestCost = costField_[idx];
            int bestDx = 0, bestDy = 0;

            for (int d = 0; d < 8; ++d) {
                const int nx = int(x) + DX[d];
                const int ny = int(y) + DY[d];
                if (nx < 0 || ny < 0 || size_t(nx) >= w_ || size_t(ny) >= h_) continue;

                if (d >= 4) {
                    if (map.isSolid(int(x) + DX[d], int(y)) || map.isSolid(int(x), int(y) + DY[d]))
                        continue;
                }

                const float nc = costField_[size_t(ny) * w_ + size_t(nx)];
                if (nc < bestCost) {
                    bestCost = nc;
                    bestDx = DX[d];
                    bestDy = DY[d];
                }
            }

            if (bestDx != 0 || bestDy != 0) {
                dirField_[idx] = Vector2Normalize({float(bestDx), float(bestDy)});
            }
        }
    }
}

Vector2 FlowField::direction(int x, int y) const {
    if (x < 0 || y < 0 || size_t(x) >= w_ || size_t(y) >= h_) return {0, 0};
    return dirField_[size_t(y) * w_ + size_t(x)];
}

float FlowField::cost(int x, int y) const {
    if (x < 0 || y < 0 || size_t(x) >= w_ || size_t(y) >= h_) return INF;
    return costField_[size_t(y) * w_ + size_t(x)];
}

}
