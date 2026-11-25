#ifndef SERVER_CONTROLLERS_NETWORK_CONTROLLER_H
#define SERVER_CONTROLLERS_NETWORK_CONTROLLER_H

#include <cstddef>

#include "shared/containers/FixedVector.h"
#include "shared/contracts/IController.h"
#include "shared/types/Action.h"
#include "shared/types/Ids.h"
#include "shared/types/Intent.h"

namespace server {

class NetworkController final : public shared::IController {
public:
    static constexpr float TIMEOUT_SECONDS = 0.25f;
    // Larger than MAX_ACTIONS_PER_TICK so a burst of reliable packets carries over instead of being dropped.
    static constexpr std::size_t PENDING_ACTIONS = 16;

    bool submit(shared::InputSeq seq, const shared::Controls& controls);

    // Returns false if the action is invalid or the queue is full.
    bool enqueue(const shared::Action& action);

    [[nodiscard]] shared::Intent decide(const shared::WorldView& ctx) override;

private:
    shared::Controls latest_{};
    shared::InputSeq lastSeq_{0};
    float silence_{0.0f};
    shared::FixedVector<shared::Action, PENDING_ACTIONS> pending_;
};

}

#endif
