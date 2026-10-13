#ifndef SERVER_CONTROLLERS_NETWORK_CONTROLLER_H
#define SERVER_CONTROLLERS_NETWORK_CONTROLLER_H

#include "shared/contracts/IController.h"
#include "shared/types/Ids.h"
#include "shared/types/Intent.h"

namespace server {

class NetworkController final : public shared::IController {
public:
    static constexpr float TIMEOUT_SECONDS = 0.25f;

    bool submit(shared::InputSeq seq, const shared::Intent& intent);

    [[nodiscard]] shared::Intent decide(const shared::ControlContext& ctx) override;

private:
    shared::Intent latest_{};
    shared::InputSeq lastSeq_{0};
    float silence_{0.0f};
};

}

#endif
