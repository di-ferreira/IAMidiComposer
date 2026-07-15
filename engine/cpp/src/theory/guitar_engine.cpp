// Implementation: GuitarEngine — strumming, arpeggios, power chords, riffs,
// funk chops, all seed-deterministic via std::mt19937_64.
//
// Guitar range: MIDI 40 (E2) to 84 (C6). Standard tuning open strings:
//   E2=40, A2=45, D3=50, G3=55, B3=59, E4=64
//
// Determinism: same seed → identical byte output.
#include <aimidi/theory/IGuitarEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <random>
#include <array>
#include <algorithm>
#include <unordered_map>
#include <string>
#include <cstdint>

namespace aimidi::theory {

namespace {

// ── Pitch-class lookup ──────────────────────────────────────────────
const std::unordered_map<std::string_view, int>& key_table() {
    static const std::unordered_map<std::string_view, int> k = {
        {"C",0},  {"C#",1}, {"Db",1}, {"D",2},  {"D#",3}, {"Eb",3},
        {"E",4},  {"F",5},  {"F#",6}, {"Gb",6}, {"G",7},  {"G#",8},
        {"Ab",8}, {"A",9},  {"A#",10},{"Bb",10},{"B",11},
    };
    return k;
}

int parse_root_pc(std::string_view key) {
    const auto& k = key_table();
    auto it = k.find(key);
    return it != k.end() ? it->second : 0;
}

// ── Chord-symbol parsing ────────────────────────────────────────────
// "Cmaj7" -> {root_pc=0, quality="maj7"}, "F#m" -> {root_pc=6, quality="min"}
struct ParsedChord { int root_pc; std::string quality; };

ParsedChord parse_chord(std::string_view sym) {
    if (sym.empty()) return {0, "maj"};
    std::string root(1, sym[0]);
    std::size_t pos = 1;
    if (pos < sym.size() && (sym[pos] == '#' || sym[pos] == 'b')) {
        root += sym[pos];
        ++pos;
    }
    std::string qual(sym.substr(pos));
    int pc = 0;
    auto it = key_table().find(root);
    if (it != key_table().end()) pc = it->second;
    if (qual.empty() || qual == "M")        qual = "maj";
    else if (qual == "m" || qual == "min")  qual = "min";
    else if (qual == "5")                   qual = "5";
    else if (qual == "dim")                 qual = "dim";
    else if (qual == "dim7")                qual = "dim7";
    else if (qual == "m7b5")                qual = "m7b5";
    else if (qual == "maj7")                qual = "maj7";
    else if (qual == "7")                   qual = "7";
    else if (qual == "m7")                  qual = "m7";
    else if (qual == "sus4")                qual = "sus4";
    else if (qual == "aug")                 qual = "aug";
    return {pc, qual};
}

// ── Chord-tone interval tables ──────────────────────────────────────
constexpr std::array<int, 3> kMaj    = {0, 4, 7};
constexpr std::array<int, 3> kMin    = {0, 3, 7};
constexpr std::array<int, 4> kDom7   = {0, 4, 7, 10};
constexpr std::array<int, 4> kMin7   = {0, 3, 7, 10};
constexpr std::array<int, 4> kMaj7   = {0, 4, 7, 11};
constexpr std::array<int, 4> kHalfDim= {0, 3, 6, 10};
constexpr std::array<int, 3> kDim    = {0, 3, 6};
constexpr std::array<int, 4> kDim7   = {0, 3, 6, 9};
constexpr std::array<int, 2> kPower  = {0, 7};
constexpr std::array<int, 3> kSus4   = {0, 5, 7};
constexpr std::array<int, 3> kAug    = {0, 4, 8};

std::vector<int> offsets_for(const std::string& quality) {
    if (quality == "maj")   return {kMaj.begin(),   kMaj.end()};
    if (quality == "min")   return {kMin.begin(),   kMin.end()};
    if (quality == "7")     return {kDom7.begin(),  kDom7.end()};
    if (quality == "m7")    return {kMin7.begin(),  kMin7.end()};
    if (quality == "maj7")  return {kMaj7.begin(),  kMaj7.end()};
    if (quality == "m7b5")  return {kHalfDim.begin(),kHalfDim.end()};
    if (quality == "dim")   return {kDim.begin(),   kDim.end()};
    if (quality == "dim7")  return {kDim7.begin(),  kDim7.end()};
    if (quality == "5")     return {kPower.begin(), kPower.end()};
    if (quality == "sus4")  return {kSus4.begin(),  kSus4.end()};
    if (quality == "aug")   return {kAug.begin(),   kAug.end()};
    return {kMaj.begin(),   kMaj.end()};
}

// ── Guitar range ────────────────────────────────────────────────────
constexpr bool in_guitar_range(int note) {
    return note >= 40 && note <= 84;
}

constexpr int kGuitarLow  = 40;  // E2
constexpr int kGuitarHigh = 84;  // C6

// Build chord-tone MIDI notes in guitar range.
// Places root in lower register (40-52), others spread upward.
std::vector<int> build_guitar_voicing(int root_pc, const std::string& quality, std::mt19937_64&) {
    auto offsets = offsets_for(quality);
    if (offsets.empty()) return {};

    std::vector<int> notes;
    notes.reserve(offsets.size());

    // Try a root-position voicing starting at MIDI 40 + root_pc.
    int base = kGuitarLow + (root_pc % 12);
    int octave = 0;
    for (std::size_t i = 0; i < offsets.size(); ++i) {
        int n = base + offsets[i] + octave * 12;
        // If note goes above range, drop an octave.
        while (n > kGuitarHigh && octave > -2) {
            --octave;
            n = base + offsets[i] + octave * 12;
        }
        // If note is below range, shift up.
        while (n < kGuitarLow) {
            n += 12;
        }
        // Keep octave for next note
        if (i + 1 < offsets.size()) {
            int next = base + offsets[i + 1] + octave * 12;
            while (next > kGuitarHigh + 12 && octave > -2) {
                octave -= 1;
            }
        }
        if (in_guitar_range(n)) {
            auto it = std::find(notes.begin(), notes.end(), n);
            if (it == notes.end()) {
                notes.push_back(n);
            }
        }
    }

    // Fallback: at least root + fifth/generic interval
    if (notes.empty()) {
        int root_note = kGuitarLow + (root_pc % 12) + 12;
        if (root_note > kGuitarHigh) root_note -= 12;
        if (in_guitar_range(root_note)) notes.push_back(root_note);
        int fifth = root_note + 7;
        if (fifth > kGuitarHigh) fifth -= 12;
        if (in_guitar_range(fifth)) notes.push_back(fifth);
    }

    // Sort low-to-high (string order for strumming).
    std::sort(notes.begin(), notes.end());
    return notes;
}

// Build power chord notes (root + fifth).
std::vector<int> build_power_voicing(int root_pc, std::mt19937_64& rng) {
    (void)rng;
    int root_note = 40 + (root_pc % 12) + 12; // aim for A2-D3 range
    if (root_note > 57) root_note -= 12;
    if (root_note < 40) root_note += 12;
    int fifth = root_note + 7;
    if (fifth > 64) { fifth -= 12; root_note -= 12; }
    if (!in_guitar_range(root_note)) root_note = 45;
    if (!in_guitar_range(fifth))     fifth = root_note + 7;
    return {root_note, fifth};
}

// Build funk chord voicing (tight, top 4 strings range).
std::vector<int> build_funk_voicing(int root_pc, const std::string& quality, std::mt19937_64&) {
    auto offsets = offsets_for(quality);
    if (offsets.empty()) offsets = {0, 4, 7};

    std::vector<int> notes;
    // Funk voicings sit in 52-76 range (D3-E5-ish, top strings).
    int base = 55 + (root_pc % 12); // start around G3
    if (base > 72) base -= 12;
    if (base < 52) base += 12;
    for (int off : offsets) {
        int n = base + off;
        if (n > 84) n -= 12;
        if (n >= 52 && n <= 84) {
            auto it = std::find(notes.begin(), notes.end(), n);
            if (it == notes.end()) notes.push_back(n);
        }
    }
    if (notes.empty()) notes = {base, base + 4, base + 7};
    std::sort(notes.begin(), notes.end());
    return notes;
}

MidiEvent make_note(std::uint32_t tick_on, std::uint32_t tick_off,
                    std::uint8_t note, std::uint8_t velocity,
                    std::uint8_t channel = 0) {
    MidiEvent e{};
    e.tick_on      = tick_on;
    e.tick_off     = tick_off;
    e.channel      = channel;
    e.note         = note;
    e.velocity     = velocity;
    e.articulation = 0u;
    return e;
}

// ── Concrete GuitarEngine ───────────────────────────────────────────
class GuitarEngine final : public IGuitarEngine {
public:
    GuitarEngine(std::shared_ptr<IScaleProvider> scales,
                 std::shared_ptr<IChordEngine> chords)
        : scales_(std::move(scales))
        , chords_(std::move(chords))
    {}

    std::vector<MidiEvent> generate_chords(
        std::string_view key, std::string_view scale,
        const std::vector<std::string>& chord_progression,
        int bars, int bpm, GuitarStyle style, Seed seed) const override
    {
        (void)key;
        (void)scale;
        (void)bpm;
        (void)chords_;

        if (bars < 1 || chord_progression.empty()) return {};

        std::mt19937_64 rng(seed);
        std::vector<MidiEvent> out;

        const std::uint32_t ppq = static_cast<std::uint32_t>(kPpq);
        const std::uint32_t ticks_per_bar = ppq * 4u; // 4/4
        const std::uint32_t tick_per_beat = ppq;

        for (int bar = 0; bar < bars; ++bar) {
            const auto& chord_str = chord_progression[static_cast<std::size_t>(bar % chord_progression.size())];
            auto parsed = parse_chord(chord_str);
            std::uint32_t bar_start = static_cast<std::uint32_t>(bar) * ticks_per_bar;

            switch (style) {
                case GuitarStyle::strumming:
                    strum_bar(out, parsed, bar_start, ticks_per_bar, ppq, tick_per_beat, bar, rng);
                    break;
                case GuitarStyle::arpeggio:
                    arpeggio_bar(out, parsed, bar_start, ticks_per_bar, ppq, bar, rng);
                    break;
                case GuitarStyle::power_chord:
                    power_bar(out, parsed, bar_start, ticks_per_bar, tick_per_beat, rng);
                    break;
                case GuitarStyle::funk_chop:
                    funk_bar(out, parsed, bar_start, ticks_per_bar, ppq, rng);
                    break;
                default:
                    strum_bar(out, parsed, bar_start, ticks_per_bar, ppq, tick_per_beat, bar, rng);
                    break;
            }
        }
        return out;
    }

    std::vector<MidiEvent> generate_power_chords(
        const std::vector<std::string>& chord_progression,
        int bars, int bpm, Seed seed) const override
    {
        (void)bpm;
        if (bars < 1 || chord_progression.empty()) return {};

        std::mt19937_64 rng(seed);
        std::vector<MidiEvent> out;
        const std::uint32_t ppq = static_cast<std::uint32_t>(kPpq);
        const std::uint32_t ticks_per_bar = ppq * 4u;
        const std::uint32_t tick_per_beat = ppq;

        for (int bar = 0; bar < bars; ++bar) {
            const auto& chord_str = chord_progression[static_cast<std::size_t>(bar % chord_progression.size())];
            auto parsed = parse_chord(chord_str);
            std::uint32_t bar_start = static_cast<std::uint32_t>(bar) * ticks_per_bar;
            power_bar(out, parsed, bar_start, ticks_per_bar, tick_per_beat, rng);
        }
        return out;
    }

    std::vector<MidiEvent> generate_riff(
        std::string_view key, std::string_view scale_name,
        int bars, int bpm, Seed seed) const override
    {
        (void)bpm;
        if (bars < 1) return {};

        std::mt19937_64 rng(seed);
        std::vector<MidiEvent> out;
        const std::uint32_t ppq = static_cast<std::uint32_t>(kPpq);
        const std::uint32_t ticks_per_bar = ppq * 4u;
        int root_pc = parse_root_pc(key);

        // Get scale intervals.
        std::vector<int> scale_intervals;
        if (scales_) {
            auto intervals = scales_->intervals_for(scale_name, root_pc);
            scale_intervals.assign(intervals.begin(), intervals.end());
        }
        // Fallback: pentatonic minor.
        if (scale_intervals.empty()) {
            scale_intervals = {(root_pc + 0) % 12, (root_pc + 3) % 12,
                               (root_pc + 5) % 12, (root_pc + 7) % 12,
                               (root_pc + 10) % 12};
        }

        // Build scale MIDI notes in range 40-72 (lower/mid strings).
        std::vector<int> scale_notes;
        for (int oct = 3; oct <= 5; ++oct) {
            int base = oct * 12;
            for (int pc : scale_intervals) {
                int n = base + (pc % 12);
                if (n >= 40 && n <= 72) {
                    auto it = std::find(scale_notes.begin(), scale_notes.end(), n);
                    if (it == scale_notes.end()) scale_notes.push_back(n);
                }
            }
        }
        std::sort(scale_notes.begin(), scale_notes.end());
        if (scale_notes.empty()) scale_notes = {40, 45, 47, 52, 55, 57};

        for (int bar = 0; bar < bars; ++bar) {
            std::uint32_t bar_start = static_cast<std::uint32_t>(bar) * ticks_per_bar;
            std::size_t r = static_cast<std::size_t>(rng());

            // 8th-note based riff pattern.
            for (int slot = 0; slot < 8; ++slot) {
                std::uint32_t tick = bar_start + static_cast<std::uint32_t>(slot * ppq / 2);
                // Skip some slots for rhythmic interest.
                if ((slot + static_cast<int>(r)) % 3 == 0) continue;

                int note_idx = static_cast<int>((rng() + static_cast<std::uint64_t>(slot) + static_cast<std::uint64_t>(bar)) % scale_notes.size());
                int note = scale_notes[static_cast<std::size_t>(note_idx)];

                // Occasionally add a bend-up (fifth above) for color.
                int duration = ppq / 2 - 10;
                if (slot % 2 == 0) {
                    duration = ppq - 10;
                }

                std::uint8_t vel = static_cast<std::uint8_t>(70 + static_cast<int>(rng() % 36));
                out.push_back(make_note(tick, tick + static_cast<std::uint32_t>(duration),
                                       static_cast<std::uint8_t>(note), vel, 0u));
            }
        }
        return out;
    }

    std::vector<MidiEvent> generate_funk_chops(
        const std::vector<std::string>& chord_progression,
        int bars, int bpm, Seed seed) const override
    {
        (void)bpm;
        if (bars < 1 || chord_progression.empty()) return {};

        std::mt19937_64 rng(seed);
        std::vector<MidiEvent> out;
        const std::uint32_t ppq = static_cast<std::uint32_t>(kPpq);
        const std::uint32_t ticks_per_bar = ppq * 4u;

        for (int bar = 0; bar < bars; ++bar) {
            const auto& chord_str = chord_progression[static_cast<std::size_t>(bar % chord_progression.size())];
            auto parsed = parse_chord(chord_str);
            std::uint32_t bar_start = static_cast<std::uint32_t>(bar) * ticks_per_bar;
            funk_bar(out, parsed, bar_start, ticks_per_bar, ppq, rng);
        }
        return out;
    }

private:
    std::shared_ptr<IScaleProvider> scales_;
    std::shared_ptr<IChordEngine>   chords_;

    // ── Strumming ────────────────────────────────────────────────────────
    void strum_bar(std::vector<MidiEvent>& out, const ParsedChord& chord,
                   std::uint32_t bar_start, std::uint32_t,
                   std::uint32_t ppq, std::uint32_t tick_per_beat,
                   int, std::mt19937_64& rng) const
    {
        auto notes = build_guitar_voicing(chord.root_pc, chord.quality, rng);
        if (notes.empty()) return;

        int beats_per_bar = 4;
        for (int beat = 0; beat < beats_per_bar; ++beat) {
            std::uint32_t beat_tick = bar_start + static_cast<std::uint32_t>(beat) * tick_per_beat;

            bool downbeat = (beat % 2 == 0);
            std::uint8_t base_vel = downbeat ?
                static_cast<std::uint8_t>(85 + static_cast<int>(rng() % 26)) :
                static_cast<std::uint8_t>(70 + static_cast<int>(rng() % 21));

            int offset_ticks = 2 + static_cast<int>(rng() % 4);
            for (std::size_t s = 0; s < notes.size(); ++s) {
                std::uint32_t on_tick = beat_tick + static_cast<std::uint32_t>(s * static_cast<std::size_t>(offset_ticks));
                std::uint32_t off_tick = on_tick + ppq - 20;
                std::uint8_t vel = static_cast<std::uint8_t>(
                    std::min(127, base_vel + static_cast<int>(rng() % 11) - 5));
                out.push_back(make_note(on_tick, off_tick,
                                       static_cast<std::uint8_t>(notes[s]), vel, 0u));
            }
        }
    }

    // ── Arpeggio ─────────────────────────────────────────────────────────
    void arpeggio_bar(std::vector<MidiEvent>& out, const ParsedChord& chord,
                      std::uint32_t bar_start, std::uint32_t ticks_per_bar,
                       std::uint32_t, int,
                      std::mt19937_64& rng) const
    {
        auto notes = build_guitar_voicing(chord.root_pc, chord.quality, rng);
        if (notes.empty()) return;

        // Common arpeggio patterns: index into notes array.
        // Pattern A: root-3-5-8 (up)
        // Pattern B: root-5-8-3 (mixed)
        // Pattern C: 8-5-3-1 (down)
        int pattern = static_cast<int>(rng() % 3);
        std::vector<std::size_t> order;
        switch (pattern) {
            case 0: // root-3-5-8 (ascending)
                for (std::size_t i = 0; i < notes.size(); ++i) order.push_back(i);
                break;
            case 1: { // root-last-third-mid
                order.push_back(0);
                if (notes.size() > 2) order.push_back(2);
                if (notes.size() > 1) order.push_back(1);
                for (std::size_t i = 3; i < notes.size(); ++i) order.push_back(i);
                break;
            }
            case 2: // descending
                for (std::size_t i = notes.size(); i > 0; --i) order.push_back(i - 1);
                break;
        }

        std::uint32_t chord_dur = ticks_per_bar;
        std::uint32_t note_dur = chord_dur / static_cast<std::uint32_t>(std::max(std::size_t{1}, order.size()));

        for (std::size_t i = 0; i < order.size(); ++i) {
            std::uint32_t on_tick = bar_start + static_cast<std::uint32_t>(i) * note_dur;
            std::uint32_t off_tick = on_tick + note_dur - 10;
            // Bass notes (first in pattern) slightly louder.
            std::uint8_t vel = (i == 0) ?
                static_cast<std::uint8_t>(85 + static_cast<int>(rng() % 21)) :
                static_cast<std::uint8_t>(70 + static_cast<int>(rng() % 21));
            out.push_back(make_note(on_tick, off_tick,
                                   static_cast<std::uint8_t>(notes[order[i]]), vel, 0u));
        }
    }

    // ── Power chords ─────────────────────────────────────────────────────
    void power_bar(std::vector<MidiEvent>& out, const ParsedChord& chord,
                   std::uint32_t bar_start, std::uint32_t,
                   std::uint32_t tick_per_beat,
                   std::mt19937_64& rng) const
    {
        auto notes = build_power_voicing(chord.root_pc, rng);
        if (notes.size() < 2) return;

        bool palm_mute = (rng() % 2) == 0;
        std::uint32_t dur = palm_mute ? tick_per_beat / 2 : tick_per_beat - 10;

        for (int beat = 0; beat < 4; ++beat) {
            std::uint32_t beat_tick = bar_start + static_cast<std::uint32_t>(beat) * tick_per_beat;
            std::uint8_t vel = static_cast<std::uint8_t>(75 + static_cast<int>(rng() % 31));
            for (int n : notes) {
                out.push_back(make_note(beat_tick, beat_tick + dur,
                                       static_cast<std::uint8_t>(n), vel, 0u));
            }
        }
    }

    // ── Funk chops ───────────────────────────────────────────────────────
    void funk_bar(std::vector<MidiEvent>& out, const ParsedChord& chord,
                  std::uint32_t bar_start, std::uint32_t,
                  std::uint32_t ppq, std::mt19937_64& rng) const
    {
        auto notes = build_funk_voicing(chord.root_pc, chord.quality, rng);
        if (notes.empty()) return;

        std::uint32_t sixteenth = ppq / 4;
        std::size_t r = static_cast<std::size_t>(rng());

        for (int slot = 0; slot < 16; ++slot) {
            if (slot % 2 == 0) continue;
            if ((slot + static_cast<int>(r)) % 3 == 0) continue;

            std::uint32_t tick = bar_start + static_cast<std::uint32_t>(slot) * sixteenth;
            std::uint32_t dur = 1;

            std::uint8_t vel = static_cast<std::uint8_t>(60 + static_cast<int>(rng() % 31));
            for (int n : notes) {
                out.push_back(make_note(tick, tick + dur,
                                       static_cast<std::uint8_t>(n), vel, 0u));
            }
        }
    }
};

} // namespace

// ── Factory ─────────────────────────────────────────────────────────
std::unique_ptr<IGuitarEngine> make_guitar_engine(
    std::shared_ptr<IScaleProvider> scales,
    std::shared_ptr<IChordEngine> chords)
{
    return std::make_unique<GuitarEngine>(std::move(scales), std::move(chords));
}

} // namespace aimidi::theory
