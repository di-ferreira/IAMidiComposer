# Music Theory Engine (C++)
# Library: aimidi_mte
# Deterministic engine for harmony/chords/melody/rhythm/bass/drums/guitar/
# piano/strings/orchestration/humanization/midi-renderer.

# --- Source list (skeleton) -----------------------------------
set(MTE_SOURCES
    src/theory/scales.cpp
    src/theory/harmony.cpp
    src/theory/chord_engine.cpp
    src/theory/bass_engine.cpp
    src/theory/rhythm_engine.cpp
    src/theory/drum_engine.cpp
    src/theory/piano_engine.cpp
    src/theory/humanization_engine.cpp
    src/theory/music_theory_module.cpp
    src/theory/midi_renderer.cpp
    src/theory/prompt_interpreter.cpp
    src/theory/style_detection.cpp
    src/theory/input_validation.cpp
    src/theory/timeline_planner.cpp
    src/theory/blueprint_generator.cpp
)

set(MTE_HEADERS
    include/aimidi/core/ServiceLocator.hpp
    include/aimidi/core/MidiEventArena.hpp
    include/aimidi/theory/IScaleProvider.hpp
    include/aimidi/theory/IHarmonyEngine.hpp
    include/aimidi/theory/IChordEngine.hpp
    include/aimidi/theory/IBassEngine.hpp
    include/aimidi/theory/IRhythmEngine.hpp
    include/aimidi/theory/IDrumEngine.hpp
    include/aimidi/theory/IPianoEngine.hpp
    include/aimidi/theory/IHumanizationEngine.hpp
    include/aimidi/theory/IMidiRenderer.hpp
    include/aimidi/theory/Types.hpp
    include/aimidi/theory/IPromptInterpreter.hpp
    include/aimidi/theory/IStyleDetection.hpp
    include/aimidi/theory/InputValidation.hpp
    include/aimidi/theory/ITimelinePlanner.hpp
    include/aimidi/theory/IBlueprintGenerator.hpp
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

# --- CLI tool ------------------------------------------------
option(AIMIDI_BUILD_CLI "Build mte_cli command-line tool" ON)
if(AIMIDI_BUILD_CLI)
    add_executable(mte_cli tools/mte_cli.cpp)
    target_link_libraries(mte_cli PRIVATE aimidi::mte)
    target_compile_features(mte_cli PRIVATE cxx_std_20)
    install(TARGETS mte_cli RUNTIME DESTINATION bin)
endif()
