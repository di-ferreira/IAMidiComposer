#pragma once
#include <vector>

#include <juce_audio_processors/juce_audio_processors.h>

namespace aimidi::plugin {

struct PluginInfo {
    juce::String name;
    juce::String manufacturer;
    juce::String format;
    juce::String path;
    bool isInstrument;
    int numInputs;
    int numOutputs;
};

class PluginScanner {
public:
    PluginScanner();
    ~PluginScanner() = default;

    std::vector<PluginInfo> scan();
    std::vector<PluginInfo> getInstruments() const;

    static int guessGMProgram(const juce::String& pluginName);

private:
    std::vector<PluginInfo> knownPlugins_;

    void addKnownInstruments();
};

} // namespace aimidi::plugin
