// VST3 Plugin Processor (header).
//
// AudioProcessor skeleton. The class is fully inline-declared here; the .cpp
// contains the factory function and the processBlock implementation.
//
// Sprint 3 / Phase 1.4: added `IEngineBridge` (DI) and a lock-free SPSC
// `StandardMidiFifo` that drains engine-produced MIDI into the Audio Thread
// without ever allocating or locking. See standards/realtime_audio.md.
#pragma once

#include <atomic>
#include <memory>
#include <span>

#include <juce_audio_utils/juce_audio_utils.h>

#include "engine/EngineBridge.hpp"
#include "engine/MidiFifo.hpp"

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

    // ---- Engine bridge access (Message Thread only) -----------------------
    // The processor owns the bridge; UI/bridge workers call via these accessors.
    const IEngineBridge& engineBridge() const noexcept { return *engine_bridge_; }
    IEngineBridge& engineBridge() noexcept { return *engine_bridge_; }

    // ---- Stable parameter IDs (for DAW automation / preset recall) -------
    static constexpr const char* paramSeed       = "seed";
    static constexpr const char* paramDensity    = "density";
    static constexpr const char* paramEnergy     = "energy";
    static constexpr const char* paramComplexity = "complexity";
    static constexpr const char* paramBPM        = "bpm";
    static constexpr const char* paramBars       = "bars";

    // ---- APVTS -----------------------------------------------------------
    juce::AudioProcessorValueTreeState apvts_;

    // ---- State (ValueTree, serialised by DAW preset) ----------------------
    juce::ValueTree getState() const { return state_; }
    void setState(const juce::ValueTree& state) { state_ = state; }

    // ---- MIDI out enqueue (Message Thread) --------------------------------
    // Called by the engine bridge callback (or a UI-driven preview) when the
    // engine returns MIDI events. Pushes into the SPSC FIFO for the Audio
    // Thread to drain in `processBlock`. `noexcept` — push never throws; if the
    // FIFO is full the remaining events are dropped (caller may log/metric).
    void enqueueMidiEvents(std::span<const FifoMidiEvent> events) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AiMidiComposerProcessor)

private:
    // SPSC FIFO: producer = Message Thread (enqueueMidiEvents), consumer =
    // Audio Thread (processBlock). Lock-free, no allocation on either side.
    StandardMidiFifo midi_out_fifo_;

    // Engine bridge — pure interface, concrete impl injected in ctor. All
    // calls happen on the Message Thread; the Audio Thread NEVER touches this.
    std::unique_ptr<IEngineBridge> engine_bridge_;

    // Mirror of `engine_bridge_->isConnected()` for lock-free reads from the
    // Message Thread (UI). The Audio Thread does not read this.
    std::atomic<bool> engine_connected_{false};

    // Guards Message-Thread-only state swaps (e.g. swapping bridge impls or
    // rebuilding pending request data). NEVER used from the Audio Thread —
    // JUCE's CriticalSection is not lock-free and must never appear in
    // `processBlock`. It is here purely for future message-thread state
    // exchanges between the editor and a bridge worker.
    juce::CriticalSection config_lock_;

    // Persisted UI/project state — serialised via getStateInformation /
    // setStateInformation so the DAW can restore it with project presets.
    juce::ValueTree state_;
};

} // namespace aimidi::plugin
