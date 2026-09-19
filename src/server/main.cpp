#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string_view>

#include <enet/enet.h>

#include "server/ServerApp.h"

namespace {

constexpr uint16_t kDefaultPort = 1234;  ///< Port used when none is given.
constexpr size_t kDefaultPeers = 32;     ///< Peer capacity used when none is given.
constexpr uint32_t kDefaultTPS = 20;     ///< Tick rate used when none is given.

/**
 * @brief Parses a decimal integer from a command-line argument.
 *
 * @tparam T    Destination integer type.
 * @param  text Argument text.
 * @param  out  Receives the parsed value; untouched on failure.
 * @return @c true if @p text was fully consumed as a valid value of @p T.
 */
template <typename T>
bool parseArg(std::string_view text, T& out) {
    T value{};
    const auto* last = text.data() + text.size();
    const auto result = std::from_chars(text.data(), last, value);
    if (result.ec != std::errc{} || result.ptr != last) {
        return false;
    }
    out = value;
    return true;
}

/**
 * @brief RAII guard around @c enet_initialize() / @c enet_deinitialize().
 *
 * @note Drop this if ENetWrappers.h already performs global initialisation;
 *       calling @c enet_initialize() twice is not harmless.
 */
class ENetGuard {
public:
    ENetGuard() : ok_(enet_initialize() == 0) {}
    ~ENetGuard() { if (ok_) enet_deinitialize(); }

    ENetGuard(const ENetGuard&) = delete;
    ENetGuard& operator=(const ENetGuard&) = delete;

    /** @brief Returns whether ENet initialised successfully. */
    explicit operator bool() const noexcept { return ok_; }

private:
    bool ok_;
};

/** @brief Prints usage to stderr. */
void printUsage(const char* program) {
    std::cerr << "Usage: " << program << " [port] [peers] [tps]\n"
              << "  port   Port to listen on (default " << kDefaultPort << ")\n"
              << "  peers  Maximum simultaneous peers (default " << kDefaultPeers << ")\n"
              << "  tps    Target ticks per second (default " << kDefaultTPS << ")\n";
}

} // namespace

/**
 * @brief Parses the command line, then runs the server loop.
 *
 * @param argc Argument count.
 * @param argv Argument values.
 * @return @c EXIT_SUCCESS on a clean run, @c EXIT_FAILURE otherwise.
 *
 * @note The loop runs until the process is terminated; nothing calls
 *       @c stop(). See ServerApp for how to wire up a signal handler.
 */
int main(int argc, char** argv) {
    uint16_t port = kDefaultPort;
    size_t peers = kDefaultPeers;
    uint32_t tps = kDefaultTPS;

    if (argc > 4) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }
    if (argc > 1 && !parseArg(argv[1], port)) {
        std::cerr << "Invalid port: " << argv[1] << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }
    if (argc > 2 && (!parseArg(argv[2], peers) || peers == 0)) {
        std::cerr << "Invalid peer count: " << argv[2] << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }
    if (argc > 3 && (!parseArg(argv[3], tps) || tps == 0)) {
        std::cerr << "Invalid tick rate: " << argv[3] << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    const ENetGuard enet;
    if (!enet) {
        std::cerr << "Failed to initialize ENet.\n";
        return EXIT_FAILURE;
    }

    server::ServerApp app(port, peers);
    app.run(tps);

    return EXIT_SUCCESS;
}
