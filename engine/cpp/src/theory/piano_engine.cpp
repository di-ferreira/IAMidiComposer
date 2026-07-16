// Implementation: PianoEngine — two-hand piano voicings with seed-deterministic
// velocity humanization and three stylistic modes (block, arpeggio, comping).
//
// Left hand plays root + fifth in the bass register (channel 0).
// Right hand plays full chord voicing in the treble register (channel 0).
//
// Determinism: same seed -> identical output (std::mt19937_64, no time).
#include <aimidi/core/Tracy.hpp>
#include <aimidi/theory/IPianoEngine.hpp>
#include <random>
#include <array>
#include <algorithm>
#include <cstdint>

namespace aimidi::theory {

namespace {

// Quality -> semitone offsets from root (same 8 qualities as IChordEngine).
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
    if (quality == "m7b5")  return {kHalfDim.begin(),kHalfDim.end()};
    if (quality == "dim")   return {kDim.begin(),   kDim.end()};
    if (quality == "dim7")  return {kDim7.begin(),  kDim7.end()};
    return {kMaj.begin(),   kMaj.end()};
}

MidiEvent make_note(std::uint32_t tick_on, std::uint32_t tick_off,
                    std::uint8_t note, std::uint8_t velocity) {
    MidiEvent e{};
    e.tick_on      = tick_on;
    e.tick_off     = tick_off;
    e.channel      = 0;
    e.note         = note;
    e.velocity     = velocity;
    e.articulation = 0u;
    return e;
}

// Clamp MIDI note to piano range A0 (21) .. C8 (108).
constexpr bool in_piano_range(int note) {
    return note >= 21 && note <= 108;
}

} // namespace

class PianoEngine final : public IPianoEngine {
public:
    std::vector<MidiEvent> generate(const PianoRequest& req) const override {
        ZoneScoped;
        std::vector<MidiEvent> out;
        const auto& chords = req.chord_req.chords;
        const int ppq = static_cast<int>(req.chord_req.ppq);
        const int bass_base   = (req.bass_octave + 1) * 12;
        const int treble_base = (req.treble_octave + 1) * 12;

        for (std::size_t i = 0; i < chords.size(); ++i) {
            const auto& cs = chords[i];
            std::mt19937_64 rng(req.chord_req.seed ^ static_cast<std::uint64_t>(i));
            const auto offsets = offsets_for(cs.quality);

            const std::uint32_t chord_on  = cs.start_tick;
            const std::uint32_t chord_off = cs.start_tick + cs.duration_ticks;
            const int chord_dur = static_cast<int>(cs.duration_ticks);

            // Per-chord velocity seed — mix the global seed with chord index.
            const auto vel_seed = req.chord_req.seed ^ static_cast<std::uint64_t>(i);

            switch (req.style) {
                case PianoStyle::block_chords: {
                    // --- Left hand: root + fifth ---
                    std::mt19937_64 vrng(vel_seed);
                    std::uint8_t lv = static_cast<std::uint8_t>(60 + (vrng() % 21));
                    int root  = bass_base + (cs.root_pc % 12);
                    int fifth = bass_base + (cs.root_pc % 12) + 7;
                    if (in_piano_range(root))
                        out.push_back(make_note(chord_on, chord_off - 10,
                                                static_cast<std::uint8_t>(root), lv));
                    if (in_piano_range(fifth))
                        out.push_back(make_note(chord_on, chord_off - 10,
                                                static_cast<std::uint8_t>(fifth), lv));

                    // --- Right hand: full voicing ---
                    std::uint8_t rv = static_cast<std::uint8_t>(50 + (vrng() % 26));
                    for (int off : offsets) {
                        int n = treble_base + (cs.root_pc % 12) + off;
                        if (in_piano_range(n))
                            out.push_back(make_note(chord_on, chord_off - 10,
                                                    static_cast<std::uint8_t>(n), rv));
                    }
                    break;
                }

                case PianoStyle::arpeggio: {
                    // --- Left hand: root at start, fifth at midpoint ---
                    std::mt19937_64 vrng(vel_seed);
                    std::uint8_t lv = static_cast<std::uint8_t>(60 + (vrng() % 21));
                    int root  = bass_base + (cs.root_pc % 12);
                    int fifth = bass_base + (cs.root_pc % 12) + 7;
                    int half_dur = chord_dur / 2;
                    if (in_piano_range(root))
                        out.push_back(make_note(chord_on, chord_on + half_dur - 5,
                                                static_cast<std::uint8_t>(root), lv));
                    if (in_piano_range(fifth))
                        out.push_back(make_note(chord_on + half_dur, chord_off - 5,
                                                static_cast<std::uint8_t>(fifth), lv));

                    // --- Right hand: spread notes across chord duration ---
                    std::uint8_t rv = static_cast<std::uint8_t>(50 + (vrng() % 26));
                    int n_notes = static_cast<int>(offsets.size());
                    for (int j = 0; j < n_notes; ++j) {
                        int on_tick  = static_cast<int>(chord_on) + (j * chord_dur / n_notes);
                        int off_tick = on_tick + (chord_dur / n_notes) - 5;
                        int n = treble_base + (cs.root_pc % 12) + offsets[j];
                        if (in_piano_range(n))
                            out.push_back(make_note(static_cast<std::uint32_t>(on_tick),
                                                     static_cast<std::uint32_t>(off_tick),
                                                     static_cast<std::uint8_t>(n), rv));
                    }
                    break;
                }

                case PianoStyle::comping: {
                    // --- Right hand on beats 1 and 3 ---
                    std::mt19937_64 vrng(vel_seed);
                    std::uint8_t rv = static_cast<std::uint8_t>(50 + (vrng() % 26));
                    for (int beat = 0; beat < 4; beat += 2) {
                        std::uint32_t bt = chord_on + static_cast<std::uint32_t>(beat * ppq);
                        for (int off : offsets) {
                            int n = treble_base + (cs.root_pc % 12) + off;
                            if (in_piano_range(n))
                                out.push_back(make_note(bt, bt + ppq - 10,
                                                        static_cast<std::uint8_t>(n), rv));
                        }
                    }

                    // --- Left hand on beats 2 and 4 ---
                    std::uint8_t lv = static_cast<std::uint8_t>(60 + (vrng() % 21));
                    for (int beat = 1; beat < 4; beat += 2) {
                        std::uint32_t bt = chord_on + static_cast<std::uint32_t>(beat * ppq);
                        int root_note = bass_base + (cs.root_pc % 12);
                        if (in_piano_range(root_note))
                            out.push_back(make_note(bt, bt + ppq - 10,
                                                    static_cast<std::uint8_t>(root_note), lv));
                    }
                    break;
                }
            }
        }
        return out;
    }
};

std::unique_ptr<IPianoEngine> make_piano_engine() {
    return std::make_unique<PianoEngine>();
}

} // namespace aimidi::theory
