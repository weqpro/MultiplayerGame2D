#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

#include <enet/enet.h>

#include "client/ClientApp.h"

namespace {

constexpr const char* kDefaultHost = "127.0.0.1"; ///< Host used when none is given.
constexpr uint16_t kDefaultPort = 1234;           ///< Port used when none is given.
constexpr uint32_t kDefaultTPS = 60;              ///< Fixed-step rate used when none is given.
constexpr uint32_t kDefaultFPS = 120;             ///< Frame rate used when none is given.
constexpr size_t kPeerCount = 1;                  ///< A client needs one outgoing connection.

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
    std::cerr << "Usage: " << program << " [host] [port] [tps] [fps]\n"
              << "  host  Server host name or address (default " << kDefaultHost << ")\n"
              << "  port  Server port (default " << kDefaultPort << ")\n"
              << "  tps   Fixed simulation steps per second (default " << kDefaultTPS << ")\n"
              << "  fps   Frames per second (default " << kDefaultFPS << ")\n";
}

} // namespace

/**
 * @brief Parses the command line, then runs the client until the server
 *        disconnects.
 *
 * @param argc Argument count.
 * @param argv Argument values.
 * @return @c EXIT_SUCCESS on a clean run, @c EXIT_FAILURE otherwise.
 */
int main(int argc, char** argv) {
    std::string host = kDefaultHost;
    uint16_t port = kDefaultPort;
    uint32_t tps = kDefaultTPS;
    uint32_t fps = kDefaultFPS;

    if (argc > 5) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }
    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2 && !parseArg(argv[2], port)) {
        std::cerr << "Invalid port: " << argv[2] << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }
    if (argc > 3 && (!parseArg(argv[3], tps) || tps == 0)) {
        std::cerr << "Invalid tick rate: " << argv[3] << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }
    // A headless client has no vsync to pace it, so the loop must.
    if (argc > 4 && (!parseArg(argv[4], fps) || fps == 0)) {
        std::cerr << "Invalid frame rate: " << argv[4] << '\n';
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    const ENetGuard enet;
    if (!enet) {
        std::cerr << "Failed to initialize ENet.\n";
        return EXIT_FAILURE;
    }

    shared::LoopConfig cfg;
    cfg.fixedHz = tps;
    cfg.frameHz = fps;

    client::ClientApp app(std::move(host), port, kPeerCount);
    app.run(cfg);

    return EXIT_SUCCESS;
}
