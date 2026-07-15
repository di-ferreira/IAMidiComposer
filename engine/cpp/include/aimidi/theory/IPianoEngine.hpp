// Piano engine interface.
//
// Generates two-hand piano MIDI from chord symbols with deterministic
// seed-based reproducibility. Three voicing styles: block chords, arpeggio,
// and comping.
#pragma once

#include <aimidi/theory/Types.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <vector>
#include <memory>
#include <cstdint>

namespace aimidi::theory {

/// Piano voicing style.
enum class PianoStyle { block_chords, arpeggio, comping };

/// Pedal sustain control (reserved for future use — not yet implemented).
struct PedalStyle {
    bool sustain_on = true;
    int  half_pedal_ticks = 0;
};

/// Request for piano generation.
struct PianoRequest {
    ChordRequest    chord_req;     // chords to voice
    PianoStyle      style = PianoStyle::block_chords;
    PedalStyle      pedal{};
    int             bass_octave = 3;   // MIDI octave for left hand (C3=48)
    int             treble_octave = 4; // MIDI octave for right hand (C4=60)
};

/// Generates piano MIDI for both hands deterministically from chord symbols.
class IPianoEngine {
public:
    virtual ~IPianoEngine() = default;

    /// Produce MidiEvents for the given chord progression (channel 0).
    virtual std::vector<MidiEvent> generate(const PianoRequest& req) const = 0;
};

/// Factory: returns a default PianoEngine.
std::unique_ptr<IPianoEngine> make_piano_engine();

} // namespace aimidi::theory
