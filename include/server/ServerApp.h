#ifndef SERVER_APP__H_
#define SERVER_APP__H_

#include <enet/enet.h>

#include "server/WebServerContext.h"

namespace server {

class ServerApp {
public:
    ServerApp(uint16_t port, size_t peerCount) : client_(port, peerCount) {}

    void GameLoop() {
        while (1) {
            client_.handleEvents(1000);
        }
    }
private:
    WebServerContext client_;
};

}

#endif
