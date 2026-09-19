#ifndef CLIENT_APP__H_
#define CLIENT_APP__H_

#include <string>
#include <cstdint>
#include <enet/enet.h>

#include "client/WebClientContext.h"

namespace client {

class ClientApp {
public:
    // Defaults host to "127.0.0.1" (localhost) if only port is provided
    ClientApp(uint16_t port, size_t peerCount = 1);
    ClientApp(std::string hostName, uint16_t port, size_t peerCount = 1);

    void StartUp();
    void GameLoop(uint32_t targetTPS = 60);

private:
    std::string hostName_;
    uint16_t port_;
    WebClientContext client_;
    ENetPeer* peer_{nullptr};
};

} // namespace client

#endif // CLIENT_APP__H_
