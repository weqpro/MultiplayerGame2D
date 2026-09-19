#include "client/WebClientContext.h"

#include <iostream>
#include <string_view>

#include "shared/ENetWrappers.h"

namespace client {

WebClientContext::WebClientContext(size_t peerCount, size_t channelLimit) {
    host_ = shared::make_enet_host(nullptr, peerCount, channelLimit);
}

ENetPeer* WebClientContext::connect(const std::string& hostName, uint16_t port, size_t channelCount, uint32_t data) {
    ENetAddress address;
    if (enet_address_set_host(&address, hostName.c_str()) < 0) {
        std::cerr << "Failed to resolve host: " << hostName << "\n";
        return nullptr;
    }
    address.port = port;

    ENetPeer* peer = enet_host_connect(host_.get(), &address, channelCount, data);
    if (!peer) {
        std::cerr << "No available peers for initiating connection.\n";
        return nullptr;
    }

    return peer;
}

bool WebClientContext::send(ENetPeer* peer, std::string_view payload, uint8_t channelID, enet_uint32 flags) {
    if (!peer) {
        std::cerr << "Cannot send packet: peer is null.\n";
        return false;
    }

    ENetPacket* packet = enet_packet_create(payload.data(), payload.size(), flags);
    if (!packet) {
        std::cerr << "Failed to allocate ENet packet.\n";
        return false;
    }

    if (enet_peer_send(peer, channelID, packet) < 0) {
        enet_packet_destroy(packet);
        std::cerr << "Failed to enqueue packet for transmission.\n";
        return false;
    }

    return true;
}

void WebClientContext::disconnect(ENetPeer* peer, uint32_t data) {
    if (peer) {
        enet_peer_disconnect(peer, data);
    }
}

void WebClientContext::handleEvents(uint32_t timeout) {
    ENetEvent event;

    while (enet_host_service(host_.get(), &event, timeout) > 0) {
        switch (event.type) {

        case ENET_EVENT_TYPE_CONNECT: {
            char ip[64];
            enet_address_get_host_ip(&event.peer->address, ip, sizeof(ip));
            std::cout << "Successfully connected to server at " << ip << ":" << event.peer->address.port << "\n";
            break;
        }

        case ENET_EVENT_TYPE_RECEIVE: {
            shared::ENetPacketPtr packet(event.packet);
            std::string_view payload(reinterpret_cast<const char*>(packet->data), packet->dataLength);

            std::cout << "Received " << packet->dataLength << " bytes from server on channel "
                      << static_cast<int>(event.channelID) << "\n"
                      << "Payload: " << payload << "\n";
            break;
        }

        case ENET_EVENT_TYPE_DISCONNECT: {
            std::cout << "Disconnected from server.\n";
            event.peer->data = nullptr;
            break;
        }

        case ENET_EVENT_TYPE_NONE:
            break;
        }
    }
}

} // namespace client
