# Music Theory Engine — tests

# GoogleTest linking: tests need RTTI/exceptions enabled for GMock; override the
# project-wide flags on this target only.

set(MTE_TESTS
    test_theory_scales.cpp
    test_theory_harmony.cpp
)

add_executable(aimidi_mte_tests ${MTE_TESTS})
target_link_libraries(aimidi_mte_tests PRIVATE
    aimidi::mte
    GTest::gtest_main)

target_compile_features(aimidi_mte_tests PRIVATE cxx_std_20)

# Re-enable RTTI/exceptions for test-only code (gtest uses them).
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(aimidi_mte_tests PRIVATE -frtti -fexceptions)
endif()

include(GoogleTest)
gtest_discover_tests(aimidi_mte_tests)
