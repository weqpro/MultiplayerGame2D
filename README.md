# MultiplayerGame2D

A 2D multiplayer survival game built with C++23. Players connect to an authoritative server over UDP and fight off NPC enemies in a tile-based world.

## Screenshots

<!-- TODO: add screenshots -->

## Features

- **Client-server multiplayer** — UDP networking via ENet with reliable and unreliable channels
- **Tile-based world** — JSON-defined maps with a sprite atlas and per-tile collision
- **NPC AI** — enemies use flow-field pathfinding with wander/chase behavior and crowd avoidance
- **Melee combat** — attacks with arc rendering, cooldowns, and health tracking
- **Smooth camera** — dead zone, look-ahead, exponential lerp, and boundary clamping
- **Custom binary protocol** — compile-time reflection-based serialization for all network messages

## Architecture

The project builds three targets:

| Target | Description |
|--------|-------------|
| `GameShared` | Static library with shared code (entities, serialization, rendering, map) |
| `MultiplayerGame2D_Client` | Client executable |
| `MultiplayerGame2D_Server` | Server executable |

**Entity hierarchy:** `Entity` → `Agent` → `Player` / `NPC`

The server runs the authoritative simulation at 120 TPS and replicates state to clients via snapshots. Clients send controls and actions; the server validates and applies them.

## Tech Stack

| Library | Version | Purpose |
|---------|---------|---------|
| Raylib | 6.0 | 2D rendering, windowing, input |
| ENet | 1.3.18 | UDP networking |
| Boost.JSON | — | JSON map parsing |
| CMake | 3.20+ | Build system |

Raylib and ENet are fetched automatically by CMake. Boost must be installed on the system.

## Build & Run

```bash
cmake -B build
cmake --build build
```

Start the server, then connect with one or more clients:

```bash
./build/MultiplayerGame2D_Server [port] [max_peers] [tps]
./build/MultiplayerGame2D_Client [host] [port] [tps]
```

Defaults: port `1234`, host `127.0.0.1`, server TPS `120`, client TPS `60`.
