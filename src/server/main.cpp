#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <raylib.h>
#include <string_view>

#include <enet/enet.h>

#include "server/ServerApp.h"

namespace {

constexpr uint16_t kDefaultPort = 1234;
constexpr size_t kDefaultPeers = 32;
constexpr uint32_t kDefaultTPS = 120;

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

void printUsage(const char* program) {
    std::cerr << "Usage: " << program << " [port] [peers] [tps]\n"
              << "  port   Port to listen on (default " << kDefaultPort << ")\n"
              << "  peers  Maximum simultaneous peers (default " << kDefaultPeers << ")\n"
              << "  tps    Target ticks per second (default " << kDefaultTPS << ")\n";
}

} // namespace

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

    const int screenWidth = 1600;
    const int screenHeight = 450 * 2;

    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(screenWidth, screenHeight, "server");

    SetTargetFPS(240);

    const shared::ENetGuard enet;
    if (!enet) {
        std::cerr << "Failed to initialize ENet.\n";
        return EXIT_FAILURE;
    }
    server::ServerApp app(port, peers);
    app.run(tps);

    CloseWindow();

    return EXIT_SUCCESS;
}
