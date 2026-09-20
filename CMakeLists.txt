cmake_minimum_required(VERSION 3.15...3.30)

project(
  MultiplayerGame2D
  VERSION 1.0.0
  LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------------------------------------------
# Dependencies
# -----------------------------------------------------------------------------
include(FetchContent)

set(ENET_BUILD_TUTORIAL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
  enet
  GIT_REPOSITORY https://github.com/lsalzman/enet.git
  GIT_TAG        v1.3.18
  SYSTEM
)

FetchContent_Declare(
  raylib
  URL            https://github.com/raysan5/raylib/archive/refs/tags/6.0.tar.gz
  SYSTEM
)

FetchContent_MakeAvailable(enet raylib)

target_include_directories(enet INTERFACE ${enet_SOURCE_DIR}/include)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# -----------------------------------------------------------------------------
# 1. Shared Library
# -----------------------------------------------------------------------------
add_library(GameShared STATIC
  src/shared/ENetWrappers.cpp
  src/shared/WebContext.cpp
)

target_include_directories(GameShared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(GameShared PUBLIC enet)

# -----------------------------------------------------------------------------
# 2. Client Executable
# -----------------------------------------------------------------------------
add_executable(MultiplayerGame2D_Client
  src/client/ClientApp.cpp
  src/client/WebClientContext.cpp
  src/client/main.cpp
)

target_link_libraries(MultiplayerGame2D_Client PRIVATE GameShared raylib Threads::Threads)

# -----------------------------------------------------------------------------
# 3. Server Executable
# -----------------------------------------------------------------------------
add_executable(MultiplayerGame2D_Server
  src/server/ServerApp.cpp
  src/server/WebServerContext.cpp
  src/server/main.cpp
)

target_link_libraries(MultiplayerGame2D_Server PRIVATE GameShared Threads::Threads)
