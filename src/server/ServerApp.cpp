#include "server/ServerApp.h"

#include <cstddef>
#include <iostream>
#include <iterator>
#include <span>
#include <string>
#include <raylib.h>

#include "server/controllers/NetworkController.h"
#include "server/controllers/WanderController.h"
#include "server/controllers/ZombieController.h"
#include "shared/Messages.h"
#include "shared/Protocol.h"
#include "shared/Renderer.h"
#include "shared/controllers/KeyboardController.h"
#include "shared/entities/NPC.h"
#include "shared/entities/Player.h"

namespace server {
namespace {

constexpr uint32_t SHUTDOWN_DRAN_STEPS_MS = 50;

constexpr int SHUTDOWN_DRAIN_STEPS = 20;

constexpr Color PLAYER_COLORS[] = {
    DARKGRAY, MAROON, ORANGE, DARKGREEN, DARKBLUE, DARKPURPLE, DARKBROWN
};

constexpr int NPC_COUNT = 67;

constexpr float NPC_LEASH = 120.0f;

Color randomPlayerColor() {
    const int last = static_cast<int>(std::size(PLAYER_COLORS)) - 1;
    return PLAYER_COLORS[GetRandomValue(0, last)];
}

Vector2 randomPoint(int minX, int maxX, int minY, int maxY) {
    return {static_cast<float>(GetRandomValue(minX, maxX)), static_cast<float>(GetRandomValue(minY, maxY))};
}

} // namespace

ServerApp::ServerApp(uint16_t port, size_t peerCount)
    : Base(port, peerCount),
      port_(port),
      peerCount_(peerCount) {}

bool ServerApp::onStartUp() {
    std::cout << "Server listening on port " << port_
              << " (up to " << peerCount_ << " peers).\n";

    auto& host = world_.spawn<shared::Player>(shared::Body{.pos = {67.0f, 67.0f}},
                                              shared::Appearance{.color = randomPlayerColor()},
                                              "host");
    world_.setMap();
    renderer_.loadAtlas();

    constexpr float TILE = 16.0f;
    auto& [cam, camCtrl] = world_.camera();
    const auto& map = world_.worldMap();
    camCtrl.bounds = {0, 0, static_cast<float>(map.width()) * TILE, static_cast<float>(map.height()) * TILE};
    camCtrl.snapTo(cam, host.body().pos);

    world_.possess<shared::KeyboardController>(host);
    hostPawn_ = host.id();

    for (int i = 0; i < NPC_COUNT; ++i) {
        const Vector2 home = randomPoint(250, 550, 120, 330);
        auto& npc = world_.spawn<shared::NPC>(shared::Body{.pos = home},
                                              shared::Appearance{.color = GRAY, .size = {12.0f, 12.0f}});
        world_.possess<ZombieController>(npc, home, NPC_LEASH, 120.0f);
    }
    return true;
}

void ServerApp::onShutDown() {
    ENetHost* host = context_.getHost();
    if (!host) {
        return;
    }

    std::cout << "Shutting down, disconnecting " << connectedPeers_ << " peer(s).\n";

    for (ENetPeer* peer = host->peers; peer < host->peers + host->peerCount; ++peer) {
        if (peer->state == ENET_PEER_STATE_CONNECTED) {
            enet_peer_disconnect(peer, 0);
        }
    }

    for (int i = 0; i < SHUTDOWN_DRAIN_STEPS && connectedPeers_ > 0; ++i) {
        service(SHUTDOWN_DRAN_STEPS_MS);
    }

    for (ENetPeer* peer = host->peers; peer < host->peers + host->peerCount; ++peer) {
        if (peer->state != ENET_PEER_STATE_DISCONNECTED) {
            enet_peer_reset(peer);
        }
    }
    connectedPeers_ = 0;
}

void ServerApp::onConnect(ENetPeer* peer) {
    ++connectedPeers_;

    auto& pawn = world_.spawn<shared::Player>(shared::Body{.pos = randomPoint(100, 200, 100, 200)},
                                              shared::Appearance{.color = randomPlayerColor()},
                                              "n n " + std::to_string(peer->incomingPeerID + 1));
    world_.possess<NetworkController>(pawn);
    sessions_[peer] = pawn.id();
    replicator_.join(peer, pawn.id());

    std::cout << "Peer connected from " << peer->address.host << ":" << peer->address.port
              << " as entity " << pawn.id() << " (" << connectedPeers_ << " total).\n";
}

void ServerApp::onReceive(ENetPeer* peer, ENetPacket* packet, uint8_t /*channelID*/) {
    const std::span<const std::byte> data(reinterpret_cast<const std::byte*>(packet->data), packet->dataLength);
    const auto result = shared::dispatch<shared::ClientMessages>(data, [this, peer](const auto& message) {
        handle(peer, message);
    });
    if (result != shared::DispatchResult::OK) {
        //TODO: handle
        std::cout << "Bad packet (" << static_cast<int>(result) << ")" << std::endl;
    }
}

void ServerApp::handle(ENetPeer* peer, const shared::ControlsMessage& message) {
    if (auto* controller = networkControllerOf(peer)) {
        controller->submit(message.seq, message.controls);
    }
}

void ServerApp::enqueueAction(ENetPeer* peer, const shared::Action& action) {
    if (auto* controller = networkControllerOf(peer)) {
        controller->enqueue(action);
    }
}

NetworkController* ServerApp::networkControllerOf(ENetPeer* peer) {
    const auto session = sessions_.find(peer);
    if (session == sessions_.end()) {
        return nullptr;
    }
    return world_.controllerOf<NetworkController>(session->second);
}

void ServerApp::onDisconnect(ENetPeer* peer) {
    if (connectedPeers_ > 0) {
        --connectedPeers_;
    }
    std::cout << "Peer disconnected from " << peer->address.host << ":" << peer->address.port
              << " (" << connectedPeers_ << " remaining).\n";

    replicator_.leave(peer);
    const auto session = sessions_.find(peer);
    if (session == sessions_.end()) {
        return;
    }
    world_.despawn(session->second);
    sessions_.erase(session);
}

void ServerApp::onTick(uint64_t tick) {
    if (WindowShouldClose()) {
        stop();
    }
    world_.update(tickSeconds());

    auto attacks = world_.pendingActiveAttacks();
    activeAttacks_.insert(activeAttacks_.end(), attacks.begin(), attacks.end());

    replicator_.flush(world_, tick);

    for (auto& a : activeAttacks_) a.timeLeft -= tickSeconds();
    std::erase_if(activeAttacks_, [](const shared::ActiveAttack& a) { return a.timeLeft <= 0.0f; });

    auto& [cam, camCtrl] = world_.camera();
    auto me = world_.entities().find<shared::Player>(hostPawn_);
    BeginDrawing();
    ClearBackground(RAYWHITE);
    if (!me) {
        renderer_.render(world_.entities(), world_.worldMap(), cam, hostPawn_, activeAttacks_);
        EndDrawing();
        return;
    }
    camCtrl.update(cam, tickSeconds(), me->body().pos, me->body().pos - me->prevPos());
    renderer_.render(world_.entities(), world_.worldMap(), cam, hostPawn_, activeAttacks_);
#ifdef ENABLE_DEBUG_DRAW
    if (IsKeyPressed(KEY_Y)) showFlowField_ = !showFlowField_;
    if (showFlowField_) {
        BeginMode2D(cam);
        if (me) DrawCircleLinesV(me->body().pos, 120.0f, WHITE);
        constexpr int T = shared::Renderer::TILE;
        {
            Vector2 tl = GetScreenToWorld2D({0, 0}, cam);
            Vector2 br = GetScreenToWorld2D({float(GetScreenWidth()), float(GetScreenHeight())}, cam);
            int ox0 = std::max(0, int(tl.x) / T);
            int oy0 = std::max(0, int(tl.y) / T);
            int ox1 = std::min(int(world_.worldMap().width()) - 1, int(br.x) / T);
            int oy1 = std::min(int(world_.worldMap().height()) - 1, int(br.y) / T);
            const auto& occ = world_.occupancy();
            for (int y = oy0; y <= oy1; ++y) {
                for (int x = ox0; x <= ox1; ++x) {
                    uint8_t n = occ.count(x, y);
                    if (n == 0) continue;
                    unsigned char alpha = static_cast<unsigned char>(std::min(180, int(n) * 45));
                    DrawRectangle(x * T, y * T, T, T, {230, 41, 55, alpha});
                }
            }
        }
        const auto* field = world_.flowFields().fieldFor(hostPawn_);
        if (field) {
            constexpr int STEP = T / 2;
            constexpr float Tf = float(T);
            Vector2 tl = GetScreenToWorld2D({0, 0}, cam);
            Vector2 br = GetScreenToWorld2D({float(GetScreenWidth()), float(GetScreenHeight())}, cam);
            int px0 = std::max(0, int(tl.x) / STEP * STEP);
            int py0 = std::max(0, int(tl.y) / STEP * STEP);
            int px1 = std::min(int(world_.worldMap().width()) * T, int(br.x) + STEP);
            int py1 = std::min(int(world_.worldMap().height()) * T, int(br.y) + STEP);

            float maxCost = 0.0f;
            int tx0 = std::max(0, px0 / T);
            int ty0 = std::max(0, py0 / T);
            int tx1 = std::min(int(world_.worldMap().width()) - 1, px1 / T);
            int ty1 = std::min(int(world_.worldMap().height()) - 1, py1 / T);
            for (int ty = ty0; ty <= ty1; ++ty)
                for (int tx = tx0; tx <= tx1; ++tx) {
                    float c = field->cost(tx, ty);
                    if (c < 1e30f && c > maxCost) maxCost = c;
                }
            if (maxCost < 1.0f) maxCost = 1.0f;

            for (int py = py0; py < py1; py += STEP) {
                for (int px = px0; px < px1; px += STEP) {
                    float spx = float(px) + STEP / 2.0f;
                    float spy = float(py) + STEP / 2.0f;

                    float sx = spx / Tf - 0.5f;
                    float sy = spy / Tf - 0.5f;
                    int ix = int(std::floor(sx));
                    int iy = int(std::floor(sy));
                    float fx = sx - float(ix);
                    float fy = sy - float(iy);

                    Vector2 d00 = field->direction(ix, iy);
                    Vector2 d10 = field->direction(ix + 1, iy);
                    Vector2 d01 = field->direction(ix, iy + 1);
                    Vector2 d11 = field->direction(ix + 1, iy + 1);

                    Vector2 dir = {
                        d00.x * (1-fx)*(1-fy) + d10.x * fx*(1-fy) + d01.x * (1-fx)*fy + d11.x * fx*fy,
                        d00.y * (1-fx)*(1-fy) + d10.y * fx*(1-fy) + d01.y * (1-fx)*fy + d11.y * fx*fy,
                    };
                    float dlen = std::sqrt(dir.x*dir.x + dir.y*dir.y);
                    if (dlen < 1e-6f) continue;
                    dir.x /= dlen;
                    dir.y /= dlen;

                    float t = std::clamp(field->cost(px / T, py / T) / maxCost, 0.0f, 1.0f);
                    unsigned char cr, cg, cb;
                    if (t < 0.5f) {
                        float s = t * 2.0f;
                        cr = (unsigned char)(s * 255);
                        cg = (unsigned char)(200 + s * (165 - 200));
                        cb = 0;
                    } else {
                        float s = (t - 0.5f) * 2.0f;
                        cr = (unsigned char)(255 + s * (230 - 255));
                        cg = (unsigned char)(165 + s * (41 - 165));
                        cb = (unsigned char)(s * 55);
                    }
                    Color color = {cr, cg, cb, 180};

                    float len = STEP * 0.35f;
                    float ex = spx + dir.x * len;
                    float ey = spy + dir.y * len;
                    DrawLineEx({spx, spy}, {ex, ey}, 1.0f, color);

                    float nx = -dir.y, ny = dir.x;
                    float wl = STEP * 0.15f;
                    float bx = ex - dir.x * wl;
                    float by = ey - dir.y * wl;
                    DrawLineEx({ex, ey}, {bx + nx * wl, by + ny * wl}, 1.0f, color);
                    DrawLineEx({ex, ey}, {bx - nx * wl, by - ny * wl}, 1.0f, color);
                }
            }
        }
        EndMode2D();
    }
#endif
    EndDrawing();
}

} // namespace server
