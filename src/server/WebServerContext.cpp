#include "server/WebServerContext.h"

#include <iostream>

namespace server {

WebServerContext::WebServerContext(uint16_t port, size_t peerCount, size_t channelCount) {
    addr_.host = ENET_HOST_ANY;
    addr_.port = port;

    host_ = shared::make_enet_host(&addr_, peerCount, channelCount);
}

void WebServerContext::onPeerConnected(ENetPeer* peer) {
    char ip[64];
    enet_address_get_host_ip(&peer->address, ip, sizeof(ip));

    std::cout << "Client connected from " << ip << ":" << peer->address.port << "\n";
}

void WebServerContext::onPeerDisconnected(ENetPeer* /*peer*/) {}

} // namespace server
