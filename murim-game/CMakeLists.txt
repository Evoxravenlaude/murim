cmake_minimum_required(VERSION 3.15)
project(MurimGame C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find raylib
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)

# Source files
set(SOURCES
    src/main.c
    src/engine/renderer.c
    src/engine/camera.c
    src/engine/particles.c
    src/engine/weather.c
    src/game/world.c
    src/game/combat.c
    src/game/cultivation.c
    src/game/npc.c
    src/game/elements.c
    src/game/bestiary.c
    src/game/bounty.c
    src/game/environment.c
    src/game/alchemy.c
    src/game/dungeon.c
    src/game/taming.c
    src/game/standoff.c
    src/game/exploration.c
    src/game/events.c
    src/game/mounts.c
    src/game/fishing.c
    src/game/reputation.c
    src/ui/hud.c
    src/ui/system_ui.c
    # v4.0 new systems
    src/game/save.c
    src/game/audio_sys.c
    src/game/quests.c
)

# Create executable
add_executable(murim_game ${SOURCES})

# Include directories
target_include_directories(murim_game PRIVATE
    ${RAYLIB_INCLUDE_DIRS}
    src/
)

# Link libraries
target_link_libraries(murim_game
    ${RAYLIB_LIBRARIES}
)

# Link flags
target_link_options(murim_game PRIVATE ${RAYLIB_LDFLAGS_OTHER})

# macOS framework dependencies
if(APPLE)
    target_link_libraries(murim_game
        "-framework IOKit"
        "-framework Cocoa"
        "-framework OpenGL"
    )
endif()

# Set output directory
set_target_properties(murim_game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Compiler warnings
target_compile_options(murim_game PRIVATE -Wall -Wextra -Wpedantic)

message(STATUS "Raylib include: ${RAYLIB_INCLUDE_DIRS}")
message(STATUS "Raylib libs: ${RAYLIB_LIBRARIES}")
