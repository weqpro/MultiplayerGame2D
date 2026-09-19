#ifndef WEB_CLIENT_CONTEXT__H_
#define WEB_CLIENT_CONTEXT__H_

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

#endif // WEB_CLIENT_CONTEXT__H_
