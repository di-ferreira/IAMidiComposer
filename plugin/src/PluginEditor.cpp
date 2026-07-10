// VST3 Plugin Editor (skeleton).
//
// All UI runs on the Message Thread of the host. Updating from the Audio Thread
// happens via atomic dirty flags + Timer poll. No blocking calls between threads.
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.hpp"

namespace aimidi::plugin {

class AiMidiComposerEditor : public juce::AudioProcessorEditor {
public:
    explicit AiMidiComposerEditor(AiMidiComposerProcessor& p)
        : juce::AudioProcessorEditor(&p), proc_(p) {
        setSize(640, 480);
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colour::fromRGB(28, 28, 32));
        g.setColour(juce::Colour::fromRGB(230, 230, 230));
        g.setFont(20.0f);
        g.drawText("AI MIDI Composer (skeleton)", getLocalBounds(),
                   juce::Justification::centred);
    }

    void resized() override {
        // Piano roll / prompt box / workflow selector to come.
    }

private:
    AiMidiComposerProcessor& proc_;
};

juce::AudioProcessorEditor* AiMidiComposerProcessor::createEditor() {
    return new AiMidiComposerEditor(*this);
}

} // namespace aimidi::plugin
