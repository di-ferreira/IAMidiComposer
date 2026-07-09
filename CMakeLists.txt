cmake_minimum_required(VERSION 3.22)

# ──────────────────────────────────────────────────────────────
# AI MIDI Composer — top-level CMake
# Monorepo: plugin/  (VST3 JUCE)  engine/cpp/  (Music Theory Engine)
#           engine/go/ (ACE server, Go)         proto/  (gRPC+proto)
# ──────────────────────────────────────────────────────────────

project(ai_midi_composer
    VERSION 0.1.0
    DESCRIPTION "AI MIDI Composer — composicao musical assistida por IA, 100% offline."
    LANGUAGES CXX)

# ── C++20 global
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Default build type
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

# ── Output layout
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ── Position independent code (shared libs / VST3)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ── Hardening flags applied to C++ targets (testes podem override)
add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:-Wall>
    $<$<COMPILE_LANGUAGE:CXX>:-Wextra>
    $<$<COMPILE_LANGUAGE:CXX>:-Wpedantic>
    $<$<COMPILE_LANGUAGE:CXX>:-Wshadow>
    $<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>
    $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>
)

# ── Sanitizers (debug only, opt-in)
option(AIMIDI_ENABLE_ASAN "Enable AddressSanitizer on Debug" OFF)
option(AIMIDI_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer on Debug" OFF)
if((CMAKE_BUILD_TYPE STREQUAL "Debug") AND AIMIDI_ENABLE_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()
if((CMAKE_BUILD_TYPE STREQUAL "Debug") AND AIMIDI_ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=undefined)
endif()

# ── Options gate
option(AIMIDI_BUILD_PLUGIN "Build the VST3 plugin (needs JUCE)" ON)
option(AIMIDI_BUILD_ENGINE_CPP "Build Music Theory Engine (C++)" ON)
option(AIMIDI_BUILD_TESTS "Build GoogleTest tests" ON)
option(AIMIDI_BUILD_BENCHMARKS "Build benchmarks (requires Tracy ABI)" OFF)

# ── Subdirectories (ordering is intentional)
if(AIMIDI_BUILD_ENGINE_CPP)
    add_subdirectory(engine/cpp)
endif()

if(AIMIDI_BUILD_PLUGIN)
    add_subdirectory(plugin)
endif()

# ── CTest
if(AIMIDI_BUILD_TESTS)
    enable_testing()
endif()

message(STATUS "AI MIDI Composer ${PROJECT_VERSION} configured:")
message(STATUS "  build_plugin       = ${AIMIDI_BUILD_PLUGIN}")
message(STATUS "  build_engine_cpp   = ${AIMIDI_BUILD_ENGINE_CPP}")
message(STATUS "  build_tests        = ${AIMIDI_BUILD_TESTS}")
message(STATUS "  build_benchmarks   = ${AIMIDI_BUILD_BENCHMARKS}")
message(STATUS "  build_type         = ${CMAKE_BUILD_TYPE}")
