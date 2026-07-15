// Implementation: RhythmEngine — deterministic drum patterns per bar.
// Seeds: req.seed ^ bar_index ^ resolution_tag produces per-bar variation.
#include <aimidi/theory/IRhythmEngine.hpp>
#include <random>
#include <vector>
#include <cstdint>

namespace aimidi::theory {

namespace {

// GM drum notes.
constexpr std::uint8_t kKick  = 36;
constexpr std::uint8_t kSnare = 38;
constexpr std::uint8_t kClosedHH = 42;
constexpr std::uint8_t kOpenHH = 46;

// Channel 9 (0-indexed) = GM drum channel 10.
constexpr std::uint8_t kDrumChannel = 9;

// Bar length at kPpq.
constexpr std::uint32_t kBarTicks = kPpq * 4;  // 4/4 at 480 ppq = 1920

// Humanized velocity within a range.
std::uint8_t humanized_velocity(std::mt19937_64& rng,
                                int lo, int hi) {
    return static_cast<std::uint8_t>(
        std::clamp(lo + static_cast<int>(rng() % (hi - lo + 1)), lo, hi));
}

// One beat's worth of eighth-note ticks (straight), 2 positions.
// Returns 2 tick offsets within a beat.
constexpr std::array<std::uint32_t, 2> eighth_beats_straight() {
    return {0u, kPpq / 2};
}

// One beat's worth of sixteenth-note ticks, 4 positions.
constexpr std::array<std::uint32_t, 4> sixteenth_beats() {
    return {0u, kPpq / 4, kPpq / 2, 3 * kPpq / 4};
}

// Full bar of eighth-note tick offsets (4 beats × 2 = 8 positions).
std::vector<std::uint32_t> eighth_bar_ticks_straight() {
    std::vector<std::uint32_t> ticks;
    ticks.reserve(8);
    for (int beat = 0; beat < 4; ++beat) {
        for (std::uint32_t off : eighth_beats_straight()) {
            ticks.push_back(beat * kPpq + off);
        }
    }
    return ticks;
}

// Full bar of sixteenth-note tick offsets (4 beats × 4 = 16 positions).
std::vector<std::uint32_t> sixteenth_bar_ticks() {
    std::vector<std::uint32_t> ticks;
    ticks.reserve(16);
    for (int beat = 0; beat < 4; ++beat) {
        for (std::uint32_t off : sixteenth_beats()) {
            ticks.push_back(beat * kPpq + off);
        }
    }
    return ticks;
}

// Apply swing: delay even-indexed (off-beat) eighth-note positions.
// Swing is applied after we build the base tick list; swing_ticks = swing * kPpq / 4.
std::vector<std::uint32_t> apply_swing(const std::vector<std::uint32_t>& base,
                                        float swing) {
    if (swing <= 0.0f) return base;
    const int swing_delta = static_cast<int>(swing * static_cast<float>(kPpq) / 4.0f);
    std::vector<std::uint32_t> out;
    out.reserve(base.size());
    for (std::size_t i = 0; i < base.size(); ++i) {
        // Even indices (off-beats) get delayed: positions 1,3,5,7,... (0-indexed)
        // For 16th notes, "even" means every other position, so we delay those
        // whose half-beat position within the beat is odd (i.e., indices 1,3 in each beat of 4).
        const int pos_in_beat = static_cast<int>(i % 4);
        if (pos_in_beat == 1 || pos_in_beat == 3) {
            out.push_back(base[i] + swing_delta);
        } else {
            out.push_back(base[i]);
        }
    }
    return out;
}

// Populate a MidiEvent on channel 9.
MidiEvent make_drum_event(std::uint32_t tick_on, std::uint32_t tick_off,
                           std::uint8_t note, std::uint8_t velocity) {
    MidiEvent e{};
    e.tick_on     = tick_on;
    e.tick_off    = tick_off;
    e.channel     = kDrumChannel;
    e.note        = note;
    e.velocity    = velocity;
    e.articulation = 0u;
    return e;
}

// Generate a basic pattern (kick/snare/HH) for one bar of eighth notes.
// 8 positions per bar: downbeat (kick+HH) and upbeat (snare+HH).
void fill_eighth_pattern(std::vector<MidiEvent>& out,
                          std::uint32_t bar_start,
                          const std::vector<std::uint32_t>& ticks,
                          std::mt19937_64& rng) {
    for (std::size_t i = 0; i < ticks.size(); ++i) {
        const std::uint32_t t = bar_start + ticks[i];
        const bool is_downbeat = (i % 2 == 0);

        // Hi-hat on every 8th position
        out.push_back(make_drum_event(t, t + 1, kClosedHH,
                                       humanized_velocity(rng, 60, 85)));

        if (is_downbeat) {
            // Downbeat: kick
            out.push_back(make_drum_event(t, t + 1, kKick,
                                           humanized_velocity(rng, 80, 100)));
        } else {
            // Upbeat: snare
            out.push_back(make_drum_event(t, t + 1, kSnare,
                                           humanized_velocity(rng, 80, 100)));
        }
    }
}

void fill_sixteenth_pattern(std::vector<MidiEvent>& out,
                             std::uint32_t bar_start,
                             const std::vector<std::uint32_t>& ticks,
                             std::mt19937_64& rng) {
    // Same structure but on 16th grid; HH on every position.
    for (std::size_t i = 0; i < ticks.size(); ++i) {
        const std::uint32_t t = bar_start + ticks[i];
        const int pos_in_beat = static_cast<int>(i % 4);

        out.push_back(make_drum_event(t, t + 1, kClosedHH,
                                       humanized_velocity(rng, 60, 85)));

        if (pos_in_beat == 0 || pos_in_beat == 2) {
            out.push_back(make_drum_event(t, t + 1, kKick,
                                           humanized_velocity(rng, 80, 100)));
        }
        if (pos_in_beat == 1 || pos_in_beat == 3) {
            out.push_back(make_drum_event(t, t + 1, kSnare,
                                           humanized_velocity(rng, 80, 100)));
        }
    }
}

// Fill patterns inserted at end of bar.
void fill_bar_fill(std::vector<MidiEvent>& out,
                   std::uint32_t bar_start,
                   int strength,
                   std::mt19937_64& rng) {
    const std::uint32_t beat4_start = bar_start + 3 * kPpq;

    if (strength == 1) {
        // Subtle: kick + snare on beat 4 only
        out.push_back(make_drum_event(beat4_start, beat4_start + 1, kKick,
                                       humanized_velocity(rng, 85, 100)));
        out.push_back(make_drum_event(beat4_start, beat4_start + 1, kSnare,
                                       humanized_velocity(rng, 85, 100)));
    } else if (strength == 2) {
        // Medium: kick on beat 3, snare+openHH on beat 4
        const std::uint32_t beat3_start = bar_start + 2 * kPpq;
        out.push_back(make_drum_event(beat3_start, beat3_start + 1, kKick,
                                       humanized_velocity(rng, 90, 105)));
        out.push_back(make_drum_event(beat4_start, beat4_start + 1, kSnare,
                                       humanized_velocity(rng, 90, 105)));
        out.push_back(make_drum_event(beat4_start, beat4_start + 1, kOpenHH,
                                       humanized_velocity(rng, 75, 90)));
    } else if (strength >= 3) {
        // Heavy: kick on beat 3, kick+snare+openHH on beat 4 with boosted velocity
        const std::uint32_t beat3_start = bar_start + 2 * kPpq;
        out.push_back(make_drum_event(beat3_start, beat3_start + 1, kKick,
                                       humanized_velocity(rng, 95, 110)));
        out.push_back(make_drum_event(beat4_start, beat4_start + 1, kKick,
                                       humanized_velocity(rng, 95, 110)));
        out.push_back(make_drum_event(beat4_start, beat4_start + 1, kSnare,
                                       humanized_velocity(rng, 95, 110)));
        out.push_back(make_drum_event(beat4_start, beat4_start + 1, kOpenHH,
                                       humanized_velocity(rng, 85, 100)));
    }
}

} // anonymous namespace

class RhythmEngine final : public IRhythmEngine {
public:
    std::vector<MidiEvent> generate(const ChordRequest& req,
                                    const RhythmStyle& style,
                                    const FillSpec&     fill) const override {
        std::vector<MidiEvent> out;
        out.reserve(req.chords.size() * 24);

        const bool is_sixteenth = (style.resolution == RhythmResolution::sixteenth);
        const auto base_ticks = is_sixteenth ? sixteenth_bar_ticks() : eighth_bar_ticks_straight();
        const auto swung_ticks = apply_swing(base_ticks, style.swing);

        // Group chords by bar.
        // Bar index = start_tick / kBarTicks
        for (std::size_t ci = 0; ci < req.chords.size(); ++ci) {
            const auto& cs = req.chords[ci];
            const std::uint32_t bar_index = cs.start_tick / kBarTicks;

            // Seed determinism: combine request seed, bar index, resolution tag
            const Seed seed = req.seed ^ static_cast<Seed>(bar_index)
                            ^ (is_sixteenth ? 0xDEADBEEFu : 0xCAFEBABEu);
            std::mt19937_64 rng(seed);

            // Generate pattern for this bar (one chord = one bar at 4/4)
            const std::uint32_t bar_start = bar_index * kBarTicks;

            if (is_sixteenth) {
                fill_sixteenth_pattern(out, bar_start, swung_ticks, rng);
            } else {
                fill_eighth_pattern(out, bar_start, swung_ticks, rng);
            }

            // Apply fill if at the specified bar.
            if (fill.start_bar >= 0 && static_cast<std::uint32_t>(fill.start_bar) == bar_index) {
                fill_bar_fill(out, bar_start, fill.strength, rng);
            }

            // Advance ci to end of bar (skip any chords fully inside this bar)
            // A chord's start_tick should mark the bar; we generate one pattern per bar.
            // The input may have one chord per bar (4 beats each).
            (void)ci; // suppress unused warning
        }

        return out;
    }
};

std::unique_ptr<IRhythmEngine> make_rhythm_engine() {
    return std::make_unique<RhythmEngine>();
}

} // namespace aimidi::theory