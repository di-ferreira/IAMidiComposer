# VST3 Plugin (C++20 + JUCE)
#
# Fetches JUCE via FetchContent and builds the plugin target as VST3 (and
# AU/CLAP variants). Disabled by CMake option AIMIDI_BUILD_PLUGIN.
#
# Requires: CMake 3.22+, C++20, network access to fetch JUCE.

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 7.0.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(ai_midi_composer
    COMPANY_NAME        "AI MIDI Composer"
    PLUGIN_MANUFACTURER_CODE    AmC
    PLUGIN_CODE                Aimd
    FORMATS                    VST3
    PRODUCT_NAME               "AI MIDI Composer"
    IS_SYNTH                   FALSE
    NEEDS_MIDI_INPUT           TRUE
    NEEDS_MIDI_OUTPUT          TRUE
    COPY_PLUGIN_AFTER_BUILD    TRUE
    NEEDS_CURL                 FALSE)

target_sources(ai_midi_composer PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/PluginEditor.hpp
    src/ui/PianoRollComponent.cpp
    src/ui/PresetManager.cpp
    src/engine/StubEngineBridge.cpp
    src/engine/PluginScanner.cpp)

target_compile_definitions(ai_midi_composer PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_ALSA=0
    JUCE_JACK=0)

target_compile_features(ai_midi_composer PRIVATE cxx_std_20)

target_include_directories(ai_midi_composer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(ai_midi_composer PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
    PUBLIC
    juce_recommended_config_flags
    juce_recommended_lto_flags
    juce_recommended_warning_flags)
