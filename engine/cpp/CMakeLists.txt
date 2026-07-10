# Music Theory Engine (C++)
# Library: aimidi_mte
# Deterministic engine for harmony/chords/melody/rhythm/bass/drums/guitar/
# piano/strings/orchestration/humanization/midi-renderer.

# --- Source list (skeleton) -----------------------------------
set(MTE_SOURCES
    src/theory/scales.cpp
    src/theory/harmony.cpp
    src/theory/chord_engine.cpp
    src/theory/music_theory_module.cpp
)

set(MTE_HEADERS
    include/aimidi/core/ServiceLocator.hpp
    include/aimidi/theory/IScaleProvider.hpp
    include/aimidi/theory/IHarmonyEngine.hpp
    include/aimidi/theory/IChordEngine.hpp
    include/aimidi/theory/Types.hpp
)

# --- Library target ------------------------------------------
add_library(aimidi_mte STATIC ${MTE_SOURCES} ${MTE_HEADERS})
add_library(aimidi::mte ALIAS aimidi_mte)

target_include_directories(aimidi_mte PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_compile_features(aimidi_mte PUBLIC cxx_std_20)

# Hardening: no exceptions / no RTTI on the standard library path.
# Tests may re-enable by overriding via target options.
target_compile_options(aimidi_mte PRIVATE
    $<$<CONFIG:Release>:-O3>
    $<$<CONFIG:Release>:-DNDEBUG>
)

# --- Tests ---------------------------------------------------
if(AIMIDI_BUILD_TESTS)
    # GoogleTest via FetchContent
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG v1.14.0
        GIT_SHALLOW TRUE
    )
    # On MSVC avoid gtest_override
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    add_subdirectory(tests)
endif()
