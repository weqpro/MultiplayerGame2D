#include "shared/WebContext.h"

namespace shared::network {

bool WebContext::send(ENetPeer* peer, std::span<const std::byte> payload, uint8_t channelID, enet_uint32 flags) {
    if (!peer) return false;

    ENetPacket* packet = enet_packet_create(payload.data(), payload.size(), flags);
    if (!packet) return false;

    if (enet_peer_send(peer, channelID, packet) < 0) {
        enet_packet_destroy(packet);
        return false;
    }

    return true;
}

void WebContext::broadcast(std::span<const std::byte> payload, uint8_t channelID, enet_uint32 flags) {
    if (!host_) return;

    ENetPacket* packet = enet_packet_create(payload.data(), payload.size(), flags);
    if (!packet) return;

    enet_host_broadcast(host_.get(), channelID, packet);
}

}
