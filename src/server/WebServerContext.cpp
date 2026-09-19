#include "server/WebServerContext.h"

#include <iostream>

#include "shared/ENetWrappers.h"

namespace server {

WebServerContext::WebServerContext(uint16_t port, size_t peerCount) {
    addr_.host = ENET_HOST_ANY;
    addr_.port = port;

    host_ = shared::make_enet_host(&addr_, peerCount, 2);
}

void WebServerContext::handleEvents(uint32_t timeout) {
    ENetEvent event;

    while (enet_host_service(host_.get(), &event, timeout) > 0) {
        switch (event.type) {

        case ENET_EVENT_TYPE_CONNECT: {
            char ip[64];
            enet_address_get_host_ip(&event.peer->address, ip, sizeof(ip));

            std::cout << "Client connected from " << ip << ":" << event.peer->address.port << "\n";

            // Attach dynamic context/session data to the peer
            auto* session = new PeerSession{ "Client_" + std::to_string(event.peer->connectID) };
            event.peer->data = session;
            break;
        }

        case ENET_EVENT_TYPE_RECEIVE: {
            shared::ENetPacketPtr packet(event.packet);

            auto* session = static_cast<PeerSession*>(event.peer->data);
            std::string_view payload(reinterpret_cast<const char*>(packet->data), packet->dataLength);

            std::cout << "Received " << packet->dataLength << " bytes from "
                      << (session ? session->client_id : "Unknown")
                      << " on channel " << static_cast<int>(event.channelID) << "\n"
                      << "Payload: " << payload << "\n";

            break;
        }

        case ENET_EVENT_TYPE_DISCONNECT: {
            auto* session = static_cast<PeerSession*>(event.peer->data);
            if (session) {
                std::cout << session->client_id << " disconnected.\n";
                delete session;
                event.peer->data = nullptr;
            }
            break;
        }

        case ENET_EVENT_TYPE_NONE:
            break;
        }
    }
}

}
