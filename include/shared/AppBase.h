#ifndef APP_BASE__H_
#define APP_BASE__H_

#include <chrono>
#include <cstdint>
#include <thread>
#include <utility>

#include <enet/enet.h>

namespace shared {

/**
 * @brief Timing parameters for AppBase::run().
 */
struct LoopConfig {
    /** @brief Clock driving the loop; mirrors AppBase::Clock. */
    using Duration = std::chrono::steady_clock::duration;

    /**
     * @brief Fixed-step (simulation) rate in steps per second.
     *
     * Each step advances the simulation by exactly @c 1/fixedHz. A value of 0
     * is clamped to 1.
     */
    uint32_t fixedHz = 60;

    /**
     * @brief Loop-paced frame rate in frames per second.
     *
     * When non-zero the loop sleeps until the next frame boundary after each
     * iteration. When zero the loop never sleeps: frames run back-to-back and
     * pacing is left to an external source such as a vsync'd buffer swap.
     *
     * @warning A headless application with @c frameHz @c == @c 0 spins at
     *          100% CPU.
     */
    uint32_t frameHz = 0;

    /**
     * @brief Upper bound on the frame time fed into the accumulator.
     *
     * A frame that measures longer than this (debugger pause, @c SIGSTOP,
     * heavy hitch) is clamped, so at most
     * @c ceil(maxFrameDelta/fixedDt)+1 fixed steps can run in one frame.
     * The simulation falls behind wall-clock time instead of spiralling.
     * Equivalent to Unity's @c Time.maximumDeltaTime.
     */
    Duration maxFrameDelta = std::chrono::milliseconds(250);
};

/**
 * @brief CRTP application skeleton owning the network context and a
 *        fixed-timestep loop with a variable-rate frame on top.
 *
 * The loop follows the standard accumulator pattern: real time elapsed since
 * the previous frame is added to an accumulator, and the simulation is
 * stepped by a constant @c fixedDt as many whole times as the accumulator
 * holds. The remainder, expressed as @c alpha @c = @c accumulator/fixedDt in
 * @c [0,1), is handed to the renderer so it can interpolate between the last
 * two fixed states. The ordering within one frame matches Unity:
 *
 * 1. @c poll()                     - pump the network once per frame
 * 2. @c onFixedUpdate(tick) x0..N  - simulation / physics / network tick
 * 3. @c onUpdate(frameDelta)       - variable-rate per-frame work
 * 4. @c onRender(alpha)            - presentation, interpolated
 *
 * Derived classes customise behaviour by shadowing any of the hooks below;
 * dispatch is static, so a hook is picked up only if its signature matches.
 *
 * Lifecycle hooks (may be private in @p Derived if it befriends this base):
 * - @c bool @c onStartUp()
 * - @c void @c onFixedUpdate(uint64_t)
 * - @c void @c onUpdate(Duration)
 * - @c void @c onRender(double)
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
    /** @brief Clock driving the loop. */
    using Clock = std::chrono::steady_clock;

    /** @brief Integer time span of @c Clock; all loop timing uses this type. */
    using Duration = Clock::duration;

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
     * @brief Runs the application: start-up, loop, shut-down.
     *
     * Each iteration measures the time since the previous frame, clamps it to
     * @c cfg.maxFrameDelta, and adds it to the accumulator. The network is
     * pumped once via @c poll(), then @c onFixedUpdate() runs once per whole
     * @c fixedDt held by the accumulator, then @c onUpdate() and
     * @c onRender() run once. If @c cfg.frameHz is non-zero the loop finally
     * sleeps until the next frame boundary; a frame that overruns its budget
     * drops the backlog rather than accumulating it.
     *
     * The loop exits once stop() is called or @c onStartUp() fails;
     * @c onShutDown() runs in either case.
     *
     * @param cfg Timing parameters; see LoopConfig.
     *
     * @note @c fixedDt @c = @c 1s/fixedHz is computed in integer @c Duration
     *       units, so it may be short by less than one unit per step. Every
     *       peer using the same @c fixedHz shares the same @c fixedDt and
     *       therefore the same (sub-microsecond-per-second) bias.
     */
    void run(const LoopConfig& cfg) {
        const uint32_t fixedHz = cfg.fixedHz == 0 ? 1 : cfg.fixedHz;
        fixedDt_ = std::chrono::duration_cast<Duration>(std::chrono::seconds(1)) / fixedHz;

        const Duration frameInterval =
            cfg.frameHz == 0
                ? Duration::zero()
                : std::chrono::duration_cast<Duration>(std::chrono::seconds(1)) / cfg.frameHz;

        if (!self().onStartUp()) {
            self().onShutDown();
            return;
        }

        running_ = true;

        Duration accumulator = Duration::zero();
        auto previous = Clock::now();
        auto nextFrame = previous + frameInterval;

        while (running_) {
            const auto now = Clock::now();
            Duration frameDt = now - previous;
            previous = now;

            if (frameDt > cfg.maxFrameDelta) {
                droppedTime_ += frameDt - cfg.maxFrameDelta; ///< Hitch: give up sim time, not stability.
                frameDt = cfg.maxFrameDelta;
            }
            frameDt_ = frameDt;
            accumulator += frameDt;

            self().poll();
            if (!running_) {
                break;
            }

            while (accumulator >= fixedDt_ && running_) {
                self().onFixedUpdate(tick_);
                ++tick_;
                accumulator -= fixedDt_;
            }
            if (!running_) {
                break;
            }

            alpha_ = static_cast<double>(accumulator.count()) / static_cast<double>(fixedDt_.count());

            self().onUpdate(frameDt);
            self().onRender(alpha_);
            ++frame_;

            if (frameInterval > Duration::zero()) {
                nextFrame += frameInterval;
                const auto afterFrame = Clock::now();
                if (nextFrame <= afterFrame) {
                    nextFrame = afterFrame; ///< Running behind: drop the backlog.
                } else {
                    std::this_thread::sleep_until(nextFrame);
                }
            }
        }

        self().onShutDown();
    }

    /**
     * @brief Requests termination of the loop.
     *
     * The loop exits at the next check: between fixed steps, before
     * @c onUpdate(), or at the top of the next iteration.
     *
     * @warning @c running_ is a plain @c bool. Make it @c std::atomic<bool>
     *          before calling this from another thread, or
     *          @c volatile @c sig_atomic_t before calling it from a signal
     *          handler.
     */
    void stop() noexcept { running_ = false; }

    /** @brief Returns whether the loop is currently running. */
    bool isRunning() const noexcept { return running_; }

    /** @brief Returns the number of fixed steps completed so far. */
    uint64_t tickCount() const noexcept { return tick_; }

    /** @brief Returns the number of frames completed so far. */
    uint64_t frameCount() const noexcept { return frame_; }

    /** @brief Returns the constant time span of one fixed step. */
    Duration fixedDelta() const noexcept { return fixedDt_; }

    /** @brief Returns the (clamped) real time consumed by the current frame. */
    Duration frameDelta() const noexcept { return frameDt_; }

    /**
     * @brief Returns the interpolation factor of the current frame.
     *
     * @c 0 means the render happens exactly at the last fixed state; values
     * approach @c 1 as the render nears the next fixed step. Renderers should
     * blend @c previous and @c current fixed state by this amount.
     */
    double alpha() const noexcept { return alpha_; }

    /** @brief Returns the total time discarded by @c maxFrameDelta clamping. */
    Duration droppedTime() const noexcept { return droppedTime_; }

    /** @brief Converts a @c Duration to floating-point seconds. */
    static float seconds(Duration d) noexcept {
        return std::chrono::duration<float>(d).count();
    }

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
     * @brief Fixed-step hook: simulation, physics, network tick.
     *
     * Runs zero or more times per frame, each call advancing the simulation
     * by exactly fixedDelta(). Do not read the wall clock here; use
     * @p tick and fixedDelta().
     *
     * @param tick Zero-based index of the current fixed step.
     */
    void onFixedUpdate(uint64_t /*tick*/) {}

    /**
     * @brief Per-frame hook: variable-rate work such as input sampling,
     *        animation and UI.
     *
     * Runs exactly once per frame, after the fixed steps of that frame.
     *
     * @param frameDelta Real time consumed by this frame, after clamping.
     */
    void onUpdate(Duration /*frameDelta*/) {}

    /**
     * @brief Render hook, called once per frame after @c onUpdate().
     *
     * @param alpha Interpolation factor in @c [0,1) between the previous and
     *              current fixed state; see alpha().
     */
    void onRender(double /*alpha*/) {}

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
     * @brief Pumps the network once per frame, before the fixed steps.
     *
     * Override to take full control of how events are serviced.
     */
    void poll() { self().service(self().serviceTimeout()); }

    /**
     * @brief Milliseconds @c poll() may block inside @c enet_host_service().
     *
     * @return 0 by default (non-blocking poll).
     *
     * @note The blocking time counts toward the frame and is absorbed by the
     *       subsequent sleep, so a timeout larger than the frame interval
     *       effectively caps the frame rate.
     */
    uint32_t serviceTimeout() const { return 0; }

private:
    /** @brief Downcasts to the concrete application type. */
    Derived& self() noexcept { return static_cast<Derived&>(*this); }

    /** @brief Downcasts to the concrete application type (const overload). */
    const Derived& self() const noexcept { return static_cast<const Derived&>(*this); }

    uint64_t tick_{0};                          ///< Number of fixed steps completed.
    uint64_t frame_{0};                         ///< Number of frames completed.
    Duration fixedDt_{Duration::zero()};        ///< Time span of one fixed step.
    Duration frameDt_{Duration::zero()};        ///< Clamped real time of the current frame.
    Duration droppedTime_{Duration::zero()};    ///< Total time discarded by clamping.
    double alpha_{0.0};                         ///< Interpolation factor of the current frame.
    bool running_{false};                       ///< Whether the loop is active.
};

} // namespace shared

#endif // APP_BASE__H_
