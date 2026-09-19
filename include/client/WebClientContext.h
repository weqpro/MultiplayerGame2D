#ifndef WEB_CLIENT_CONTEXT__H_
#define WEB_CLIENT_CONTEXT__H_

#include <string>
#include <string_view>
#include <enet/enet.h>

#include "shared/ENetWrappers.h"

namespace client {

class WebClientContext {
public:
    explicit WebClientContext(size_t peerCount = 1, size_t channelLimit = 2);

    ENetPeer* connect(const std::string& hostName, uint16_t port, size_t channelCount = 2, uint32_t data = 0);

    bool send(ENetPeer* peer, std::string_view payload, uint8_t channelID = 0, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);

    void disconnect(ENetPeer* peer, uint32_t data = 0);

    void handleEvents(uint32_t timeout = 0);

private:
    shared::ENetHostPtr host_;
};

} // !namespace client

#endif // WEB_CLIENT_CONTEXT__H_
