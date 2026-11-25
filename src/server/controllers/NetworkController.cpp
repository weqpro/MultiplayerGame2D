#include "server/controllers/NetworkController.h"

namespace server {

bool NetworkController::submit(shared::InputSeq seq, const shared::Controls& controls) {
    if (seq <= lastSeq_) {
        return false;
    }
    lastSeq_ = seq;
    latest_ = shared::sanitize(controls); // TODO: not sanitize twice
    silence_ = 0.0f;
    return true;
}

bool NetworkController::enqueue(const shared::Action& action) {
    const auto sanitized = shared::sanitize(action);
    return sanitized && pending_.push_back(*sanitized);
}

shared::Intent NetworkController::decide(const shared::ControlContext& ctx) {
    silence_ += ctx.dt;

    shared::Intent intent;
    if (silence_ <= TIMEOUT_SECONDS) {
        intent.controls = latest_;
    }

    std::size_t taken = 0;
    for (const shared::Action& action : pending_) {
        if (!intent.actions.push_back(action)) {
            break;
        }
        ++taken;
    }
    pending_.dropFront(taken);
    return intent;
}

}
