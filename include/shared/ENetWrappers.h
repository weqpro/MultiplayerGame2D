#ifndef WRAPPERS__H_
#define WRAPPERS__H_

#include <enet/enet.h>
#include <memory>

namespace server {

// ENetHost *
struct ENetHostDeleter {
    void operator()(ENetHost *host) const noexcept;
};

using ENetHostPtr = std::unique_ptr<ENetHost, ENetHostDeleter>;

ENetHostPtr make_enet_host(const ENetAddress *address, size_t peerCount, 
                           size_t channelLimit, enet_uint32 incomingBandwidth = 0, 
                           enet_uint32 outgoingBandwidth = 0);

// ENetPacket *
struct ENetPacketDeleter {
    void operator()(ENetPacket* packet) const noexcept {
        if (packet) enet_packet_destroy(packet);
    }
};
using ENetPacketPtr = std::unique_ptr<ENetPacket, ENetPacketDeleter>;

}

#endif
