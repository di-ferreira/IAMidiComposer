// VST3 Plugin Processor — implementation.
//
// Scope: plug is a co-pilot; audio buffers are zeroed and MIDI is fed out via a
// lock-free SPSC FIFO (drianed in processBlock). All AI/large work happens in
// the ACE Engine via the IEngineBridge (now stubbed; gRPC client after ADR-0004).
//
// Realtime contract (inviolable): processBlock performs NO heap allocation, NO
// lock, NO IO, NO AI call, NO future.get(). It only pops from the pre-allocated
// SPSC `midi_out_fifo_` and writes into the host's `MidiBuffer`.
#include "PluginProcessor.hpp"

#include <algorithm>

namespace aimidi::plugin {

AiMidiComposerProcessor::AiMidiComposerProcessor()
    : juce::AudioProcessor(BusesProperties()
          .withOutput("Output", juce::AudioChannelSet::stereo(), true))
    , apvts_(*this, nullptr, "Parameters",
        {
            std::make_unique<juce::AudioParameterInt>(
                paramSeed, "Seed", 0, 999999, 42),
            std::make_unique<juce::AudioParameterFloat>(
                paramDensity, "Density", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>(
                paramEnergy, "Energy", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterFloat>(
                paramComplexity, "Complexity", 0.0f, 1.0f, 0.5f),
            std::make_unique<juce::AudioParameterInt>(
                paramBPM, "BPM", 60, 200, 120),
            std::make_unique<juce::AudioParameterInt>(
                paramBars, "Bars", 1, 64, 8),
        }) {
    // Message Thread: heap allocation of the stub bridge is permitted here.
    engine_bridge_ = make_stub_engine_bridge();
    state_ = juce::ValueTree("AiMidiComposerState");
}

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

void AiMidiComposerProcessor::prepareToPlay(double, int) {
    // Not the Audio Thread (JUCE guarantees prepareToPlay runs on a non-RT
    // thread before playback starts). Safe to clear the FIFO.
    midi_out_fifo_.clear();
}

void AiMidiComposerProcessor::releaseResources() {
    // Same contract as prepareToPlay — non-RT thread.
    midi_out_fifo_.clear();
}

void AiMidiComposerProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midi) {
    // ---- Audio Thread: NO alloc, NO lock, NO IO, NO AI -------------------
    buffer.clear();
    juce::ScopedNoDenormals noDenormals;

    // Drain the SPSC FIFO into the host's MidiBuffer. `pop` is noexcept and
    // lock-free; the loop terminates when the FIFO is empty.
    const auto num_samples = static_cast<std::size_t>(buffer.getNumSamples());
    FifoMidiEvent ev;
    while (midi_out_fifo_.pop(ev)) {
        // Clamp sample_position to [0, num_samples-1]; if num_samples == 0 we
        // skip (defensive — host should never call with an empty block).
        if (num_samples == 0) {
            break;
        }
        const auto pos = static_cast<int>(
            std::min<std::size_t>(ev.sample_position, num_samples - 1));

        const auto juce_channel = static_cast<int>(ev.channel) + 1; // JUCE channels are 1..16
        if (ev.is_note_on) {
            midi.addEvent(juce::MidiMessage::noteOn(
                              juce_channel,
                              static_cast<int>(ev.note),
                              static_cast<juce::uint8>(ev.velocity)),
                          pos);
        } else {
            midi.addEvent(juce::MidiMessage::noteOff(
                              juce_channel,
                              static_cast<int>(ev.note)),
                          pos);
        }
    }
    // NOTE: engine_bridge_ is intentionally NOT touched here — all bridge
    // calls live on the Message Thread.
}

void AiMidiComposerProcessor::enqueueMidiEvents(
    std::span<const FifoMidiEvent> events) noexcept {
    // Message Thread (or bridge worker). Push is lock-free and noexcept; on
    // FIFO-full we drop the rest. In production we'd log + increment a metric
    // from here (allowed — never called from the Audio Thread).
    for (const auto& ev : events) {
        if (!midi_out_fifo_.push(ev)) {
            break; // FIFO full — drop remainder
        }
    }
}

bool AiMidiComposerProcessor::hasEditor() const { return true; }
// createEditor() is implemented in PluginEditor.cpp.

void AiMidiComposerProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts_.copyState();
    auto editorCopy = state_.createCopy();
    if (editorCopy.isValid())
        state.addChild(editorCopy, -1, nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary(*xml, destData);
}

void AiMidiComposerProcessor::setStateInformation(const void* data, int sizeInBytes) {
    if (auto xml = getXmlFromBinary(data, sizeInBytes)) {
        auto state = juce::ValueTree::fromXml(*xml);
        if (state.isValid()) {
            apvts_.replaceState(state);
            auto editorChild = state.getChildWithName("AiMidiComposerState");
            if (editorChild.isValid())
                state_ = editorChild;
        }
    }
}

bool AiMidiComposerProcessor::canAddBus(bool) const { return false; }
bool AiMidiComposerProcessor::isBusesLayoutSupported(const BusesLayout&) const { return true; }

} // namespace aimidi::plugin

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new aimidi::plugin::AiMidiComposerProcessor();
}
