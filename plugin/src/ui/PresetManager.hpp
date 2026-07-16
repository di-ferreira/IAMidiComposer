#pragma once
#include <map>
#include <vector>

#include <juce_audio_utils/juce_audio_utils.h>

namespace aimidi::plugin {

struct Preset {
    juce::String name;
    juce::String category;
    juce::String tags;
    juce::ValueTree state;
};

class PresetManager {
public:
    PresetManager();
    ~PresetManager() = default;

    void loadPresets();
    void savePreset(const Preset& preset);
    void deletePreset(const juce::String& name);

    juce::StringArray getPresetNames() const;
    juce::StringArray getPresetNamesByCategory(const juce::String& category) const;
    juce::StringArray getPresetNamesByTag(const juce::String& tag) const;

    Preset getPreset(const juce::String& name) const;
    bool hasPreset(const juce::String& name) const;

    static std::vector<Preset> createFactoryPresets();

private:
    juce::File getPresetsDirectory() const;
    std::map<juce::String, Preset> presets_;
};

} // namespace aimidi::plugin
