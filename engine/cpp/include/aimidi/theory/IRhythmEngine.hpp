// Rhythm engine interface.
//
// Generates rhythmic accompaniment (drums) seeded deterministically per bar.
// All timing in PPQ ticks; GM drums on channel 9 (0-indexed).
#pragma once

#include <aimidi/theory/Types.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <vector>
#include <memory>
#include <string_view>
#include <cstdint>

namespace aimidi::theory {

// Grid resolution for rhythm generation.
enum class RhythmResolution { eighth, sixteenth };

// Swing amount: 0.0 = straight, 1.0 = full swing (eighth notes delayed).
struct RhythmStyle {
    RhythmResolution resolution = RhythmResolution::eighth;
    float             swing     = 0.0f;   // 0..1
    Seed              seed      = 0;
};

// Fills are optional rhythmic breaks at section boundaries.
struct FillSpec {
    int start_bar = -1;   // -1 = no fill
    int strength  = 1;   // 1=subtle, 2=medium, 3=heavy
};

class IRhythmEngine {
public:
    virtual ~IRhythmEngine() = default;

    // Given a sequence of ChordSpecs (start_tick, duration_ticks, root_pc, quality),
    // emit MidiEvents representing the rhythmic accompaniment for that harmonic context.
    // channel: 9 (GM drums channel 10 is 0-indexed as 9)
    // req.seed deterministically picks which rhythmic variant to use per bar.
    virtual std::vector<MidiEvent> generate(
        const ChordRequest& req,
        const RhythmStyle&  style,
        const FillSpec&     fill) const = 0;
};

std::unique_ptr<IRhythmEngine> make_rhythm_engine();

} // namespace aimidi::theory