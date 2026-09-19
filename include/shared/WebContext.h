#ifndef WEB_CONTEXT__H_
#define WEB_CONTEXT__H_

#include <enet/enet.h>
#include <string_view>
#include "shared/ENetWrappers.h"

namespace shared::network {

class WebContext {
public:
    virtual ~WebContext() = default;

    template <typename Handler> // TODO: concept of handler
    void handleEvents(Handler& handler, uint32_t timeout = 0) {
        ENetEvent event;

        while (enet_host_service(host_.get(), &event, timeout) > 0) {
            switch (event.type) {

            case ENET_EVENT_TYPE_CONNECT:
                onPeerConnected(event.peer);
                handler.onConnect(event.peer);
                break;

            case ENET_EVENT_TYPE_RECEIVE: {
                ENetPacketPtr packet(event.packet);
                handler.onReceive(event.peer, packet.get(), event.channelID);
                break;
            }

            case ENET_EVENT_TYPE_DISCONNECT:
                onPeerDisconnected(event.peer);
                handler.onDisconnect(event.peer);
                break;

            case ENET_EVENT_TYPE_NONE:
                break;
            }
        }
    }

    bool send(ENetPeer* peer, std::string_view payload, uint8_t channelID = 0, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);
    void broadcast(std::string_view payload, uint8_t channelID = 0, enet_uint32 flags = ENET_PACKET_FLAG_RELIABLE);

    ENetHost* getHost() const { return host_.get(); }

protected:
    ENetHostPtr host_;

    virtual void onPeerConnected(ENetPeer* peer) {}
    virtual void onPeerDisconnected(ENetPeer* peer) {}
};

} // namespace network

#endif // WEB_CONTEXT__H_
