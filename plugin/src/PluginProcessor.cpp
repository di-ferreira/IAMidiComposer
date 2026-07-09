// VST3 Plugin Processor — implementation.
//
// Scope: plug is a co-pilot; audio buffers are zeroed and MIDI is fed out via
// lock-free FIFO (TODO). All AI/large work happens in the ACE Engine via gRPC.
#include "PluginProcessor.hpp"

namespace aimidi::plugin {

AiMidiComposerProcessor::AiMidiComposerProcessor()
    : juce::AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)) {}

AiMidiComposerProcessor::~AiMidiComposerProcessor() = default;

const juce::String AiMidiComposerProcessor::getName() const { return "AI MIDI Composer"; }
bool AiMidiComposerProcessor::acceptsMidi() const { return true; }
bool AiMidiComposerProcessor::producesMidi() const { return true; }
bool AiMidiComposerProcessor::isMidiEffect() const { return false; }

double AiMidiComposerProcessor::getTailLengthSeconds() const { return 0.0; }
int AiMidiComposerProcessor::getNumPrograms() { return 1; }
int AiMidiComposerProcessor::getCurrentProgram() { return 0; }
void AiMidiComposerProcessor::setCurrentProgram(int) {}
const juce::String AiMidiComposerProcessor::getProgramName(int) { return {}; }
void AiMidiComposerProcessor::changeProgramName(int, const juce::String&) {}

void AiMidiComposerProcessor::prepareToPlay(double, int) {}
void AiMidiComposerProcessor::releaseResources() {}

void AiMidiComposerProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midi) {
    buffer.clear();
    juce::ScopedNoDenormals noDenormals;
    // TODO: drain midi_out_fifo_ (lock-free SPSC) and feed to `midi`.
    (void)midi;
}

bool AiMidiComposerProcessor::hasEditor() const { return true; }
// createEditor() is implemented in PluginEditor.cpp.

void AiMidiComposerProcessor::getStateInformation(juce::MemoryBlock&) {}
void AiMidiComposerProcessor::setStateInformation(const void*, int) {}

bool AiMidiComposerProcessor::canAddBus(bool) const { return false; }
bool AiMidiComposerProcessor::isBusesLayoutSupported(const BusesLayout&) const { return true; }

} // namespace aimidi::plugin

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new aimidi::plugin::AiMidiComposerProcessor();
}
