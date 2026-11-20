#ifndef SERVER_WEB_SERVER_CONTEXT_H
#define SERVER_WEB_SERVER_CONTEXT_H

#include <cstdint>
#include <enet/enet.h>

#include "shared/Messages.h"
#include "shared/Protocol.h"
#include "shared/WebContext.h"

namespace server {

class WebServerContext : public shared::network::WebContext {
public:
    explicit WebServerContext(uint16_t port, size_t peerCount = 32, size_t channelCount = 2);

    using WebContext::send;
    using WebContext::broadcast;

    template <shared::Message M>
    bool send(ENetPeer* peer, const M& message) {
        return sendMessage<shared::ServerMessages>(peer, message);
    }

    template <shared::Message M>
    bool broadcast(const M& message) {
        return broadcastMessage<shared::ServerMessages>(message);
    }

protected:
    void onPeerConnected(ENetPeer* peer) override;
    void onPeerDisconnected(ENetPeer* peer) override;

private:
    ENetAddress addr_{};
};

} // namespace server

#endif // SERVER_WEB_SERVER_CONTEXT_H
