#ifndef WEB_CONTEXT__H_

#include <string>

#include <enet/enet.h>

#include "shared/ENetWrappers.h"

namespace server {

struct PeerSession {
    std::string client_id;
};

class WebServerContext {
public:
    WebServerContext(uint16_t port, size_t peerCount = 32);

    void handleEvents(uint32_t timeout);

private:
    ENetHostPtr host_;
    ENetAddress addr_;
};

}

#endif // !WEB_CONTEXT__H_
