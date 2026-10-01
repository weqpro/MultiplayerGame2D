#include <charconv>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <string>
#include <string_view>

#include <enet/enet.h>

#include "client/ClientApp.h"

namespace {

constexpr const char* kDefaultHost = "127.0.0.1";
constexpr uint16_t kDefaultPort = 1234;
constexpr uint32_t kDefaultTPS = 60;
constexpr size_t kPeerCount = 1;

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
    std::cerr << "Usage: " << program << " [host] [port] [tps]\n"
              << "  host  Server host name or address (default " << kDefaultHost << ")\n"
              << "  port  Server port (default " << kDefaultPort << ")\n"
              << "  tps   Target ticks per second (default " << kDefaultTPS << ")\n";
}

} // namespace

int main(int argc, char** argv) {
    std::string host = kDefaultHost;
    uint16_t port = kDefaultPort;
    uint32_t tps = kDefaultTPS;

    if (argc > 4) {
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

    const shared::ENetGuard enet;
    if (!enet) {
        std::cerr << "Failed to initialize ENet.\n";
        return EXIT_FAILURE;
    }

    const int screenWidth = 800;
    const int screenHeight = 450;

    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(screenWidth, screenHeight, "client");

    SetTargetFPS(240);

    client::ClientApp app(std::move(host), port, kPeerCount);
    app.run(tps);

    CloseWindow();
    return EXIT_SUCCESS;
}
