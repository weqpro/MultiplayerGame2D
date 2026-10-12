#ifndef SHARED_APP_BASE_H
#define SHARED_APP_BASE_H

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <utility>
#include <unordered_map>

#include <enet/enet.h>

#include "shared/entities/Entity.h"

namespace shared {

template <typename Derived, typename Context>
class AppBase {
public:
    using Clock = std::chrono::steady_clock;

    template <typename... Args>
    explicit AppBase(Args&&... args) : context_(std::forward<Args>(args)...) {}

    AppBase(const AppBase&) = delete;
    AppBase& operator=(const AppBase&) = delete;

    void run(uint32_t targetTPS = 60) {
        if (targetTPS == 0) {
            targetTPS = 1;
        }

        const auto tickInterval =
            std::chrono::duration_cast<Clock::duration>(std::chrono::seconds(1)) / targetTPS;

        if (!self().onStartUp()) {
            self().onShutDown();
            return;
        }

        running_ = true;
        auto nextTick = Clock::now() + tickInterval;

        while (running_) {
            self().poll();

            if (!running_) {
                break;
            }

            self().onTick(tick_);
            ++tick_;

            nextTick += tickInterval;
            const auto now = Clock::now();
            if (nextTick <= now) {
                nextTick = now;
            } else {
                std::this_thread::sleep_until(nextTick);
            }
        }

        self().onShutDown();
    }

    void stop() noexcept { running_ = false; }

    bool isRunning() const noexcept { return running_; }

    uint64_t tickCount() const noexcept { return tick_; }

    Context& context() noexcept { return context_; }

    const Context& context() const noexcept { return context_; }

    void onConnect(ENetPeer* /*peer*/) {}

    void onReceive(ENetPeer* /*peer*/, ENetPacket* /*packet*/, uint8_t /*channelID*/) {}

    void onDisconnect(ENetPeer* /*peer*/) {}

protected:
    ~AppBase() = default;

    Context context_;
    std::unordered_map<uint32_t, std::unique_ptr<shared::Entity>> entities_ {};

    uint32_t addEntity(std::unique_ptr<shared::Entity> entity) {
        static uint32_t counter = 0;
        entities_[counter++] = std::move(entity);        
        return counter;
    }

    bool onStartUp() { return true; }

    void onTick(uint64_t /*tick*/) {}

    void onShutDown() {}

    void service(uint32_t timeout) { context_.handleEvents(self(), timeout); }

    void poll() { self().service(self().serviceTimeout()); }

    uint32_t serviceTimeout() const { return 0; }

private:
    Derived& self() noexcept { return static_cast<Derived&>(*this); }

    const Derived& self() const noexcept { return static_cast<const Derived&>(*this); }

    uint64_t tick_{0};
    bool running_{false};
};

} // namespace shared

#endif // SHARED_APP_BASE_H
