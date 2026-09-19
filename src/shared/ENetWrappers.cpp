#include "shared/ENetWrappers.h"

#include <stdexcept>

namespace server {

void ENetHostDeleter::operator()(ENetHost* host) const noexcept {
    if (host) {
        enet_host_destroy(host);
    }
}

ENetHostPtr make_enet_host(const ENetAddress* address, size_t peerCount, 
                           size_t channelLimit, enet_uint32 incomingBandwidth, 
                           enet_uint32 outgoingBandwidth) {
    ENetHost* raw_host = enet_host_create(address, peerCount, channelLimit, 
                                         incomingBandwidth, outgoingBandwidth);
    
    if (!raw_host) {
        throw std::runtime_error("An error occurred while trying to create an ENet host.");
    }

    return ENetHostPtr(raw_host);
}

}
