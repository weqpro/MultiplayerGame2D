#ifndef CLIENT_WEB_CLIENT_CONTEXT_H
#define CLIENT_WEB_CLIENT_CONTEXT_H

#include <string>
#include <enet/enet.h>

#include "shared/ENetWrappers.h"
#include "shared/WebContext.h"

namespace client {

class WebClientContext : public shared::network::WebContext {
public:
    explicit WebClientContext(size_t peerCount = 1) {
        host_ = shared::make_enet_host(nullptr, peerCount, 2);
    }

    ENetPeer* connect(const std::string& hostName, uint16_t port);
};

} // !namespace client

#endif // CLIENT_WEB_CLIENT_CONTEXT_H
