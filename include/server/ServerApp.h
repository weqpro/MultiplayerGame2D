#ifndef SERVER_APP__H_
#define SERVER_APP__H_

#include <enet/enet.h>

#include "server/WebServerContext.h"

namespace server {


class ServerApp {
public:
    ServerApp(uint16_t port, size_t peerCount) : server_(port, peerCount) {}

    void GameLoop() {
        server_.handleEvents(1000);
    }
private:
    WebServerContext server_;
};

}

#endif
