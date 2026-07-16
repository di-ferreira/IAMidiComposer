#include "PresetManager.hpp"

#include <juce_core/juce_core.h>

namespace aimidi::plugin {

PresetManager::PresetManager() {
    loadPresets();
}

void PresetManager::loadPresets() {
    presets_.clear();

    // Load factory presets first
    auto factory = createFactoryPresets();
    for (auto& p : factory)
        presets_[p.name] = std::move(p);

    // Load user presets from disk
    auto dir = getPresetsDirectory();
    if (!dir.exists())
        return;

    for (auto& entry : juce::RangedDirectoryIterator(dir, false, "*.preset")) {
        auto file = entry.getFile();
        auto xml = juce::XmlDocument::parse(file);
        if (xml == nullptr)
            continue;

        Preset p;
        p.name = xml->getStringAttribute("name");
        p.category = xml->getStringAttribute("category", "User");
        p.tags = xml->getStringAttribute("tags");
        p.state = juce::ValueTree::fromXml(*xml->getChildByName("state"));

        if (p.name.isNotEmpty() && p.state.isValid())
            presets_[p.name] = std::move(p);
    }
}

void PresetManager::savePreset(const Preset& preset) {
    auto dir = getPresetsDirectory();
    dir.createDirectory();

    auto xml = std::make_unique<juce::XmlElement>("preset");
    xml->setAttribute("name", preset.name);
    xml->setAttribute("category", preset.category);
    xml->setAttribute("tags", preset.tags);

    auto stateXml = preset.state.createXml();
    if (stateXml != nullptr)
        xml->addChildElement(stateXml.release());

    auto file = dir.getChildFile(preset.name + ".preset");
    file.replaceWithText(xml->toString());
    file.setReadOnly(false);

    presets_[preset.name] = preset;
}

void PresetManager::deletePreset(const juce::String& name) {
    auto it = presets_.find(name);
    if (it == presets_.end())
        return;
    if (it->second.category != "User")
        return; // Do not delete factory presets

    auto file = getPresetsDirectory().getChildFile(name + ".preset");
    file.deleteFile();
    presets_.erase(it);
}

juce::StringArray PresetManager::getPresetNames() const {
    juce::StringArray names;
    for (const auto& [key, _] : presets_)
        names.add(key);
    return names;
}

juce::StringArray PresetManager::getPresetNamesByCategory(const juce::String& category) const {
    juce::StringArray names;
    for (const auto& [key, p] : presets_)
        if (p.category == category)
            names.add(key);
    return names;
}

juce::StringArray PresetManager::getPresetNamesByTag(const juce::String& tag) const {
    juce::StringArray names;
    for (const auto& [key, p] : presets_)
        if (p.tags.contains(tag))
            names.add(key);
    return names;
}

Preset PresetManager::getPreset(const juce::String& name) const {
    auto it = presets_.find(name);
    if (it != presets_.end())
        return it->second;
    return {};
}

bool PresetManager::hasPreset(const juce::String& name) const {
    return presets_.find(name) != presets_.end();
}

juce::File PresetManager::getPresetsDirectory() const {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("ai_midi_composer")
        .getChildFile("presets");
}

std::vector<Preset> PresetManager::createFactoryPresets() {
    std::vector<Preset> presets;

    // Helper: build a parameter state ValueTree
    auto makeState = [](int seed, float density, float energy,
                        float complexity, int bpm, int bars,
                        const juce::String& key, const juce::String& scale) {
        auto s = juce::ValueTree("Parameters");
        s.setProperty("seed", seed, nullptr);
        s.setProperty("density", density, nullptr);
        s.setProperty("energy", energy, nullptr);
        s.setProperty("complexity", complexity, nullptr);
        s.setProperty("bpm", bpm, nullptr);
        s.setProperty("bars", bars, nullptr);
        s.setProperty("key", key, nullptr);
        s.setProperty("scale", scale, nullptr);
        return s;
    };

    presets.push_back({
        "Upbeat Pop", "Factory", "pop,upbeat,high-energy",
        makeState(100, 0.6f, 0.9f, 0.4f, 120, 8, "C", "major")
    });

    presets.push_back({
        "Dark Ambient", "Factory", "ambient,dark,cinematic",
        makeState(200, 0.3f, 0.2f, 0.7f, 80, 16, "A", "minor")
    });

    presets.push_back({
        "Jazz Ballad", "Factory", "jazz,ballad,smooth",
        makeState(300, 0.5f, 0.4f, 0.7f, 100, 12, "C", "major")
    });

    presets.push_back({
        "Rock Anthem", "Factory", "rock,anthem,high-energy",
        makeState(400, 0.8f, 0.9f, 0.5f, 140, 8, "E", "minor")
    });

    presets.push_back({
        "Electronic Groove", "Factory", "electronic,groove,dance",
        makeState(500, 0.5f, 0.6f, 0.6f, 128, 8, "F", "minor")
    });

    return presets;
}

} // namespace aimidi::plugin
