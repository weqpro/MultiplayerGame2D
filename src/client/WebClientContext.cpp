#include "client/WebClientContext.h"

#include "shared/ENetWrappers.h"

namespace client {

ENetPeer* WebClientContext::connect(const std::string& hostName, uint16_t port) {
    ENetAddress address;
    enet_address_set_host(&address, hostName.c_str());
    address.port = port;
    return enet_host_connect(host_.get(), &address, 2, 0);
}
} // namespace client
