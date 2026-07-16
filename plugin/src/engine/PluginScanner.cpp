#include "PluginScanner.hpp"

namespace aimidi::plugin {

PluginScanner::PluginScanner() {
    addKnownInstruments();
}

std::vector<PluginInfo> PluginScanner::scan() {
    // Stub: return the hardcoded known list.
    // In a full implementation this would use AudioPluginFormatManager
    // and KnownPluginList to scan VST3 directories.
    return knownPlugins_;
}

std::vector<PluginInfo> PluginScanner::getInstruments() const {
    std::vector<PluginInfo> instruments;
    for (const auto& p : knownPlugins_)
        if (p.isInstrument)
            instruments.push_back(p);
    return instruments;
}

int PluginScanner::guessGMProgram(const juce::String& pluginName) {
    auto lower = pluginName.toLowerCase();

    if (lower.contains("piano") || lower.contains("grand") || lower.contains("keys"))
        return 0;
    if (lower.contains("organ") || lower.contains("b3") || lower.contains("hammond"))
        return 16;
    if (lower.contains("guitar") || lower.contains("acoustic") || lower.contains("strat"))
        return 24;
    if (lower.contains("bass") || lower.contains("fretless"))
        return 32;
    if (lower.contains("string") || lower.contains("violin") || lower.contains("cello")
        || lower.contains("viola") || lower.contains("orchestra"))
        return 48;
    if (lower.contains("synth") || lower.contains("analog") || lower.contains("lead")
        || lower.contains("pad") || lower.contains("wavetable"))
        return 80;
    if (lower.contains("brass") || lower.contains("trumpet") || lower.contains("trombone")
        || lower.contains("horn"))
        return 56;
    if (lower.contains("flute") || lower.contains("clarinet") || lower.contains("oboe")
        || lower.contains("bassoon"))
        return 73;
    if (lower.contains("drum") || lower.contains("percussion") || lower.contains("kit"))
        return 118;

    return 0;
}

void PluginScanner::addKnownInstruments() {
    knownPlugins_.clear();

    knownPlugins_.push_back({
        "Dexed", "Digital Suburban", "VST3", {}, true, 0, 2
    });

    knownPlugins_.push_back({
        "OB-Xd", "discoDSP", "VST3", {}, true, 0, 2
    });

    knownPlugins_.push_back({
        "TAL-NoiseMaker", "TAL-Togu Audio Line", "VST3", {}, true, 0, 2
    });

    knownPlugins_.push_back({
        "Surge XT", "Surge Synth Team", "VST3", {}, true, 0, 2
    });

    knownPlugins_.push_back({
        "Vital", "Matt Tytel", "VST3", {}, true, 0, 2
    });

    knownPlugins_.push_back({
        "Piano One", "Neo Waves", "VST3", {}, true, 0, 2
    });

    knownPlugins_.push_back({
        "Spitfire LABS", "Spitfire Audio", "VST3", {}, true, 0, 2
    });

    knownPlugins_.push_back({
        "DrumGizmo", "DrumGizmo", "VST3", {}, true, 0, 2
    });

    knownPlugins_.push_back({
        "Sfizz", "sfztools", "VST3", {}, true, 0, 2
    });

    knownPlugins_.push_back({
        "Helm", "Matt Tytel", "VST3", {}, true, 0, 2
    });
}

} // namespace aimidi::plugin
