# Music Theory Engine — tests

# GoogleTest linking: tests need RTTI/exceptions enabled for GMock; override the
# project-wide flags on this target only.

set(MTE_TESTS
    bench_mte_pipeline.cpp
    test_gm_programs.cpp
    test_theory_scales.cpp
    test_theory_harmony.cpp
    test_chord_engine.cpp
    test_bass_engine.cpp
    test_rhythm_engine.cpp
    test_drum_engine.cpp
    test_guitar_engine.cpp
    test_strings_engine.cpp
    test_piano_engine.cpp
    test_humanization_engine.cpp
    test_midi_renderer.cpp
    test_theory_golden.cpp
    test_service_locator.cpp
    test_arena.cpp
    test_prompt_interpreter.cpp
    test_style_detection.cpp
    test_input_validation.cpp
    test_timeline_planner.cpp
    test_blueprint_generator.cpp
    test_arrangement_planner.cpp
    test_golden_prompts.cpp
    test_instrument_mapper.cpp
    test_counterpoint_engine.cpp
    test_modulation_engine.cpp
    test_orchestration_engine.cpp
    test_reharmonize_engine.cpp
    test_orchestrate_workflow.cpp
    test_dsp.cpp
    test_key_detection.cpp
    test_chord_detection.cpp
)

add_executable(aimidi_mte_tests ${MTE_TESTS})
target_link_libraries(aimidi_mte_tests PRIVATE
    aimidi::mte
    GTest::gtest_main
    GTest::gmock)

target_compile_features(aimidi_mte_tests PRIVATE cxx_std_20)

# Re-enable RTTI/exceptions for test-only code (gtest uses them).
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(aimidi_mte_tests PRIVATE -frtti -fexceptions)
endif()

include(GoogleTest)
gtest_discover_tests(aimidi_mte_tests)
