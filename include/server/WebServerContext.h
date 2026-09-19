#ifndef WEB_SERVER_CONTEXT__H_
#define WEB_SERVER_CONTEXT__H_

#include <cstdint>
#include <string>
#include <enet/enet.h>

#include "shared/WebContext.h"

namespace server {

struct PeerSession {
    std::string client_id;
};

class WebServerContext : public shared::network::WebContext {
public:
    explicit WebServerContext(uint16_t port, size_t peerCount = 32, size_t channelCount = 2);

protected:
    void onPeerConnected(ENetPeer* peer) override;
    void onPeerDisconnected(ENetPeer* peer) override;

private:
    ENetAddress addr_{};
};

} // namespace server

#endif // !WEB_SERVER_CONTEXT__H_
