cmake_minimum_required(VERSION 3.20...3.30)

project(
  MultiplayerGame2D
  VERSION 1.0.1
  LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 23)
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

set_target_properties(enet raylib glfw PROPERTIES EXPORT_COMPILE_COMMANDS OFF)

target_include_directories(enet INTERFACE ${enet_SOURCE_DIR}/include)

if(WIN32)
  # enet/win32.h includes <winsock2.h> that collides with raylib.h.
  target_compile_definitions(enet INTERFACE WIN32_LEAN_AND_MEAN NOGDI NOUSER)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
find_package(Boost REQUIRED COMPONENTS json)

# -----------------------------------------------------------------------------
# 1. Shared Library
# -----------------------------------------------------------------------------
add_library(GameShared STATIC
  src/shared/controllers/KeyboardController.cpp
  src/shared/entities/Agent.cpp
  src/shared/entities/Entity.cpp
  src/shared/entities/EntityFactory.cpp
  src/shared/entities/EntityRegistry.cpp
  src/shared/types/Action.cpp
  src/shared/types/Intent.cpp
  src/shared/ENetWrappers.cpp
  src/shared/WebContext.cpp
  src/shared/Renderer.cpp
  src/shared/WorldMap.cpp
  src/server/CollisionQuery.cpp
)

target_include_directories(GameShared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(GameShared PUBLIC enet raylib Boost::json)

# -----------------------------------------------------------------------------
# 2. Client Executable
# -----------------------------------------------------------------------------
add_executable(MultiplayerGame2D_Client
  src/client/ClientApp.cpp
  src/client/WebClientContext.cpp
  src/client/World.cpp
  src/client/main.cpp
)

target_link_libraries(MultiplayerGame2D_Client PRIVATE GameShared raylib Threads::Threads)

# -----------------------------------------------------------------------------
# 3. Server Executable
# -----------------------------------------------------------------------------
add_executable(MultiplayerGame2D_Server
  src/server/controllers/NetworkController.cpp
  src/server/controllers/WanderController.cpp
  src/server/controllers/ZombieController.cpp
  src/server/Replicator.cpp
  src/server/ServerApp.cpp
  src/server/WebServerContext.cpp
  src/server/World.cpp
  src/server/main.cpp
)

target_link_libraries(MultiplayerGame2D_Server PRIVATE GameShared raylib Threads::Threads)

add_custom_command(TARGET MultiplayerGame2D_Server POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          ${CMAKE_SOURCE_DIR}/assets
          $<TARGET_FILE_DIR:MultiplayerGame2D_Server>/assets
)
