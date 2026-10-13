#ifndef SERVER_REPLICATOR_H
#define SERVER_REPLICATOR_H

#include <cstdint>
#include <vector>

#include <enet/enet.h>

#include "server/WebServerContext.h"
#include "server/World.h"
#include "shared/types/Ids.h"

namespace server {

class Replicator {
public:
    explicit Replicator(WebServerContext& net, uint32_t snapshotEveryTicks = 1);

    void join(ENetPeer* peer, shared::EntityId pawn);
    void leave(ENetPeer* peer);

    void flush(World& world, uint64_t tick);

private:
    struct Joiner {
        ENetPeer* peer;
        shared::EntityId pawn;
    };

    void replicateEvents(World& world);
    void bootstrapJoiners(const World& world);
    void sendSnapshot(const World& world, uint64_t tick);

    WebServerContext& net_;
    uint32_t periodTicks_;
    std::vector<Joiner> joiners_;
};

}

#endif // SERVER_REPLICATOR_H
