#ifndef APP_BASE__H_
#define APP_BASE__H_

#include <chrono>
#include <cstdint>
#include <thread>
#include <utility>

#include <enet/enet.h>

namespace shared {

/**
 * @brief CRTP application skeleton owning the network context, the fixed-rate
 *        loop and the tick timing.
 *
 * Derived classes customise behaviour by shadowing any of the hooks below;
 * dispatch is static, so a hook is picked up only if its signature matches.
 *
 * Lifecycle hooks (may be private in @p Derived if it befriends this base):
 * - @c bool @c onStartUp()
 * - @c void @c onTick(uint64_t)
 * - @c void @c onShutDown()
 * - @c uint32_t @c serviceTimeout() @c const
 * - @c void @c poll()
 *
 * Handler callbacks (must be @b public in @p Derived):
 * - @c void @c onConnect(ENetPeer*)
 * - @c void @c onReceive(ENetPeer*, @c ENetPacket*, @c uint8_t)
 * - @c void @c onDisconnect(ENetPeer*)
 *
 * @tparam Derived The concrete application type.
 * @tparam Context A shared::network::WebContext subclass owned by this base.
 *
 * @note Not polymorphic: the destructor is non-virtual and protected, so
 *       instances must not be deleted through an @c AppBase pointer.
 */
template <typename Derived, typename Context>
class AppBase {
public:
    /** @brief Clock driving the tick schedule. */
    using Clock = std::chrono::steady_clock;

    /**
     * @brief Constructs the application, forwarding all arguments to the
     *        context's constructor.
     *
     * @tparam Args Context constructor argument types.
     * @param  args Arguments forwarded verbatim to @p Context.
     */
    template <typename... Args>
    explicit AppBase(Args&&... args) : context_(std::forward<Args>(args)...) {}

    AppBase(const AppBase&) = delete;
    AppBase& operator=(const AppBase&) = delete;

    /**
     * @brief Runs the application: start-up, fixed-rate loop, shut-down.
     *
     * Each iteration pumps the network via @c poll(), then invokes
     * @c onTick() and sleeps until the next tick boundary. If a tick overruns
     * its budget the backlog is dropped rather than accumulated, so the loop
     * cannot spiral. The loop exits once stop() is called or @c onStartUp()
     * fails; @c onShutDown() runs in either case.
     *
     * @param targetTPS Target ticks per second; a value of 0 is clamped to 1.
     */
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
                nextTick = now; ///< Running behind: drop the backlog.
            } else {
                std::this_thread::sleep_until(nextTick);
            }
        }

        self().onShutDown();
    }

    /**
     * @brief Requests termination of the loop.
     *
     * The current iteration finishes first; the loop exits before the next
     * @c onTick().
     *
     * @warning @c running_ is a plain @c bool. Make it @c std::atomic<bool>
     *          before calling this from another thread, or
     *          @c volatile @c sig_atomic_t before calling it from a signal
     *          handler.
     */
    void stop() noexcept { running_ = false; }

    /** @brief Returns whether the loop is currently running. */
    bool isRunning() const noexcept { return running_; }

    /** @brief Returns the number of ticks completed so far. */
    uint64_t tickCount() const noexcept { return tick_; }

    /** @brief Returns the owned network context. */
    Context& context() noexcept { return context_; }

    /** @brief Returns the owned network context (const overload). */
    const Context& context() const noexcept { return context_; }

    /**
     * @brief Called when a peer connects. Default implementation does nothing.
     * @param peer The peer that connected.
     *
     * @note Public because @c WebContext::handleEvents() invokes the handler
     *       callbacks from outside this class; protected access would not
     *       compile.
     */
    void onConnect(ENetPeer* /*peer*/) {}

    /**
     * @brief Called when a packet arrives. Default implementation does nothing.
     *
     * @param peer      The sending peer.
     * @param packet    The received packet.
     * @param channelID The channel the packet arrived on.
     *
     * @warning The packet is owned by an @c ENetPacketPtr inside
     *          @c handleEvents() and is destroyed as soon as this call
     *          returns. Copy anything that must outlive it.
     */
    void onReceive(ENetPeer* /*peer*/, ENetPacket* /*packet*/, uint8_t /*channelID*/) {}

    /**
     * @brief Called when a peer disconnects. Default implementation does nothing.
     * @param peer The peer that disconnected.
     *
     * @note Distinct from @c WebContext::onPeerDisconnected(), the virtual
     *       context hook, which fires first for the same event.
     */
    void onDisconnect(ENetPeer* /*peer*/) {}

protected:
    /** @brief Protected and non-virtual: deletion through @c AppBase* is disallowed. */
    ~AppBase() = default;

    /** @brief The owned network context. */
    Context context_;

    /**
     * @brief Start-up hook, called once before the loop begins.
     * @return @c true to enter the loop, @c false to abort.
     *         Aborting still runs @c onShutDown().
     */
    bool onStartUp() { return true; }

    /**
     * @brief Per-tick hook, called once per iteration after the network is pumped.
     * @param tick Zero-based index of the current tick.
     */
    void onTick(uint64_t /*tick*/) {}

    /** @brief Shut-down hook, called once after the loop ends. */
    void onShutDown() {}

    /**
     * @brief Pumps the network once, dispatching events to @p Derived's
     *        handler callbacks.
     *
     * Also usable outside the loop, for example to drain a disconnect
     * handshake from @c onShutDown().
     *
     * @param timeout Milliseconds @c enet_host_service() may block; 0 polls.
     */
    void service(uint32_t timeout) { context_.handleEvents(self(), timeout); }

    /**
     * @brief Pumps the network for one loop iteration.
     *
     * Override to take full control of how events are serviced.
     */
    void poll() { self().service(self().serviceTimeout()); }

    /**
     * @brief Milliseconds @c poll() may block inside @c enet_host_service().
     *
     * @return 0 by default (non-blocking poll).
     *
     * @note The blocking time is absorbed by the subsequent sleep, so a
     *       timeout larger than the tick interval effectively caps the tick
     *       rate.
     */
    uint32_t serviceTimeout() const { return 0; }

private:
    /** @brief Downcasts to the concrete application type. */
    Derived& self() noexcept { return static_cast<Derived&>(*this); }

    /** @brief Downcasts to the concrete application type (const overload). */
    const Derived& self() const noexcept { return static_cast<const Derived&>(*this); }

    uint64_t tick_{0};   ///< Number of ticks completed.
    bool running_{false}; ///< Whether the loop is active.
};

} // namespace shared

#endif // APP_BASE__H_
