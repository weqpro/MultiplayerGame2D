#include "server/controllers/NetworkController.h"
#include "shared/types/Intent.h"

namespace server {

bool NetworkController::submit(shared::InputSeq seq, const shared::Intent& intent) {
    if (seq <= lastSeq_) {
        return false;
    }
    lastSeq_ = seq;
    latest_ = shared::sanitize(intent); // TODO: not sanitize twice
    silence_ = 0.0f;
    return true;
}

shared::Intent NetworkController::decide(const shared::ControlContext& ctx) {
    silence_ += ctx.dt;
    if (silence_ > TIMEOUT_SECONDS) return shared::Intent {};
    return latest_;
}

}
