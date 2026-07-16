// Implementation: ChordEngine — root-position voicings with seed-deterministic
// inversion selection and humanized velocity.
//
// Voice leading is RUDIMENTARY in this sprint (Sprint 4 / Phase 2.1):
//   - RNG seeded by req.seed picks an inversion in [0,1,2] per chord.
//   - Inversion circulates chord tones upward; root drops an octave when pushed.
//   - Full nearest-voice voice leading is deferred to Sprint 5 (IPianoEngine).
//
// Determinism: same seed → identical byte output (std::mt19937_64, no time).
#include <aimidi/core/Tracy.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <random>
#include <array>
#include <algorithm>

namespace aimidi::theory {

namespace {

// Quality → semitone offsets from root (root position, no octave).
// Bench target: <1ms for 4 sections of 4 bars (16 chords × ~4 notes).
constexpr std::array<int, 3> kMaj   = {0, 4, 7};
constexpr std::array<int, 3> kMin   = {0, 3, 7};
constexpr std::array<int, 4> kDom7  = {0, 4, 7, 10};
constexpr std::array<int, 4> kMin7  = {0, 3, 7, 10};
constexpr std::array<int, 4> kMaj7  = {0, 4, 7, 11};
constexpr std::array<int, 4> kHalfDim = {0, 3, 6, 10};
constexpr std::array<int, 3> kDim   = {0, 3, 6};
constexpr std::array<int, 4> kDim7  = {0, 3, 6, 9};

std::vector<int> offsets_for(const std::string& quality) {
    if (quality == "maj")   return {kMaj.begin(),   kMaj.end()};
    if (quality == "min")   return {kMin.begin(),   kMin.end()};
    if (quality == "7")     return {kDom7.begin(),  kDom7.end()};
    if (quality == "m7")    return {kMin7.begin(),  kMin7.end()};
    if (quality == "maj7")  return {kMaj7.begin(),  kMaj7.end()};
    if (quality == "m7b5") return {kHalfDim.begin(),kHalfDim.end()};
    if (quality == "dim")   return {kDim.begin(),   kDim.end()};
    if (quality == "dim7")  return {kDim7.begin(),  kDim7.end()};
    // Unknown quality → fallback to major triad.
    return {kMaj.begin(),   kMaj.end()};
}

// Apply an inversion by rotating chord-tone intervals upward.
// Inversion 0: root position. 1: first inversion (third on bottom). 2: second.
// Notes that "fall off" the top get wrapped down an octave, which keeps the
// rooted chord-coverage compact and deterministic.
void apply_inversion(std::vector<int>& notes, int inv) {
    if (notes.empty()) return;
    const int count = static_cast<int>(notes.size());
    inv = ((inv % count) + count) % count;
    if (inv == 0) return;
    std::rotate(notes.begin(), notes.begin() + inv, notes.end());
    // The rotated notes that belonged before the pivot drop an octave so all
    // chord tones stay contiguous (close voicing).
    for (int i = static_cast<int>(notes.size()) - inv; i < static_cast<int>(notes.size()); ++i) {
        notes[static_cast<std::size_t>(i)] -= 12;
    }
}

} // namespace

class ChordEngine final : public IChordEngine {
public:
    std::vector<MidiEvent> voicing(const ChordRequest& req) const override {
        ZoneScoped;
        std::mt19937_64 rng(req.seed);
        std::vector<MidiEvent> out;
        out.reserve(req.chords.size() * 4);

        const int base_octave_midi = 48; // root sits at MIDI 48 (C3)

        for (const auto& cs : req.chords) {
            const auto offsets = offsets_for(cs.quality);
            std::vector<int> notes;
            notes.reserve(offsets.size());
            for (int off : offsets) {
                const int pc = ((cs.root_pc % 12) + 12) % 12;
                notes.push_back(base_octave_midi + pc + off);
            }
            // Seed-deterministic inversion.
            const int inv = static_cast<int>(rng() % 3);
            apply_inversion(notes, inv);

            // Humanized velocity 50..100.
            const std::uint8_t velocity =
                static_cast<std::uint8_t>(50 + (rng() % 51));

            for (int n : notes) {
                if (n < 0 || n > 127) {
                    continue;
                }
                MidiEvent e{};
                e.tick_on     = cs.start_tick;
                e.tick_off    = cs.start_tick + cs.duration_ticks;
                e.channel     = cs.channel;
                e.note        = static_cast<std::uint8_t>(n);
                e.velocity    = velocity;
                e.articulation = 0u;
                out.push_back(e);
            }
        }
        return out;
    }
};

std::unique_ptr<IChordEngine> make_chord_engine() {
    return std::make_unique<ChordEngine>();
}

} // namespace aimidi::theory
