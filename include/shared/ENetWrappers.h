#ifndef WRAPPERS__H_
#define WRAPPERS__H_

#include <enet/enet.h>
#include <memory>

namespace shared {

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
    void operator()(ENetPacket* packet) const noexcept;
};
using ENetPacketPtr = std::unique_ptr<ENetPacket, ENetPacketDeleter>;

class ENetGuard {
public:
    ENetGuard() : ok_(enet_initialize() == 0) {}
    ~ENetGuard() { if (ok_) enet_deinitialize(); }

    ENetGuard(const ENetGuard&) = delete;
    ENetGuard& operator=(const ENetGuard&) = delete;

    explicit operator bool() const noexcept { return ok_; }

private:
    bool ok_;
};

}

#endif
