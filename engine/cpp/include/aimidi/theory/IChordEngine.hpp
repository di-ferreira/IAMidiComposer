// Chord engine interface.
//
// Translates ChordSpec symbols into concrete voiced MIDI events with
// seed-deterministic voice-leading and humanized velocity.
// Pure data structures use std::string for quality (not a hot path).
#pragma once

#include <aimidi/theory/Types.hpp>
#include <vector>
#include <memory>
#include <string>
#include <cstdint>

namespace aimidi::theory {

/// A single chord to be voiced, expressed in tick/Pitch-Class/quality terms.
struct ChordSpec {
    std::uint32_t start_tick;
    std::uint32_t duration_ticks;
    int           root_pc;   // pitch class 0..11
    std::string   quality;   // "maj","min","7","m7","maj7","dim","dim7","m7b5"
    std::uint8_t  channel;
};

/// Set of chords + reproduction seed.
struct ChordRequest {
    std::vector<ChordSpec> chords;
    Seed                   seed = 0;
    std::uint32_t          ppq = 480;
};

/// Voices chord symbols into MIDI events deterministically.
class IChordEngine {
public:
    virtual ~IChordEngine() = default;

    /// Produce voiced MidiEvents for every ChordSpec in the request.
    virtual std::vector<MidiEvent> voicing(const ChordRequest& req) const = 0;
};

/// Factory: returns a default ChordEngine (root-position voicings).
std::unique_ptr<IChordEngine> make_chord_engine();

} // namespace aimidi::theory
