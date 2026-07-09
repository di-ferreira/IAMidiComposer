// VST3 Plugin Processor (header).
//
// AudioProcessor skeleton. The class is fully inline-declared here; the .cpp
// contains the factory function and the processBlock implementation.
#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

namespace aimidi::plugin {

class AiMidiComposerEditor;

class AiMidiComposerProcessor : public juce::AudioProcessor {
public:
    AiMidiComposerProcessor();
    ~AiMidiComposerProcessor() override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;

    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer,
                      juce::MidiBuffer& midi) override;

    bool hasEditor() const override;
    juce::AudioProcessorEditor* createEditor() override;

    void getStateInformation(juce::MemoryBlock& dest) override;
    void setStateInformation(const void* data, int size) override;

    bool canAddBus(bool isInput) const override;
    bool isBusesLayoutSupported(const BusesLayout&) const override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AiMidiComposerProcessor)
};

} // namespace aimidi::plugin
