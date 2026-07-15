// Implementation: StringsEngine — pads, countermelodies, and swells for
// string sections (violin, viola, cello, contrabass, full ensemble).
//
// Ranges:
//   Violin:    55-84 (G3-E6)
//   Viola:     48-76 (C3-E6)
//   Cello:     36-60 (C2-C4)
//   Contrabass: 28-48 (E1-G3)
//
// Determinism: same seed → identical byte output (std::mt19937_64).
#include <aimidi/theory/IStringsEngine.hpp>
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

// ── Pitch-class lookup (same table as harmony/guitar) ───────────────
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
    else if (qual == "dim")                 qual = "dim";
    else if (qual == "dim7")                qual = "dim7";
    else if (qual == "m7b5")                qual = "m7b5";
    else if (qual == "maj7")                qual = "maj7";
    else if (qual == "7")                   qual = "7";
    else if (qual == "m7")                  qual = "m7";
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

// ── Section ranges ──────────────────────────────────────────────────
struct SectionRange { int min; int max; std::uint8_t channel; };

SectionRange range_for(StringsSection section) {
    switch (section) {
        case StringsSection::violin:      return {55, 84, 0};
        case StringsSection::viola:       return {48, 76, 1};
        case StringsSection::cello:       return {36, 60, 2};
        case StringsSection::contrabass:  return {28, 48, 3};
        default:                          return {28, 84, 0}; // full ensemble uses channel 0
    }
}

bool in_range(int note, const SectionRange& r) {
    return note >= r.min && note <= r.max;
}

MidiEvent make_note(std::uint32_t tick_on, std::uint32_t tick_off,
                    std::uint8_t note, std::uint8_t velocity,
                    std::uint8_t channel) {
    MidiEvent e{};
    e.tick_on      = tick_on;
    e.tick_off     = tick_off;
    e.channel      = channel;
    e.note         = note;
    e.velocity     = velocity;
    e.articulation = 0u;
    return e;
}

// Build chord notes in a specific section range with close voicing.
std::vector<int> voice_in_range(int root_pc, const std::string& quality,
                                const SectionRange& range,
                                std::mt19937_64&)
{
    auto offsets = offsets_for(quality);
    if (offsets.empty()) offsets = {0, 4, 7};

    std::vector<int> notes;
    notes.reserve(offsets.size());

    // Place root near the lower third of the range.
    int root_oct = range.min / 12;
    int root_note = root_oct * 12 + (root_pc % 12);
    while (root_note < range.min + 6) root_note += 12;
    while (root_note > range.min + 18) root_note -= 12;

    for (int off : offsets) {
        int n = root_note + off;
        if (n < range.min) n += 12;
        if (n > range.max) n -= 12;
        if (in_range(n, range)) {
            auto it = std::find(notes.begin(), notes.end(), n);
            if (it == notes.end()) notes.push_back(n);
        }
    }

    if (notes.empty()) {
        int f = root_note;
        if (in_range(f, range)) notes.push_back(f);
        f = root_note + 4;
        if (in_range(f, range)) notes.push_back(f);
        f = root_note + 7;
        if (in_range(f, range)) notes.push_back(f);
    }

    std::sort(notes.begin(), notes.end());
    return notes;
}

// ── Concrete StringsEngine ──────────────────────────────────────────
class StringsEngine final : public IStringsEngine {
public:
    StringsEngine(std::shared_ptr<IScaleProvider> scales,
                  std::shared_ptr<IChordEngine> chords)
        : scales_(std::move(scales))
        , chords_(std::move(chords))
    {}

    std::vector<MidiEvent> generate_pad(
        std::string_view key, std::string_view scale,
        const std::vector<std::string>& chord_progression,
        int bars, int bpm, StringsSection section, Seed seed) const override
    {
        (void)key;
        (void)scale;
        (void)bpm;
        if (bars < 1 || chord_progression.empty()) return {};

        std::mt19937_64 rng(seed);
        std::vector<MidiEvent> out;
        const std::uint32_t ppq = static_cast<std::uint32_t>(kPpq);
        const std::uint32_t ticks_per_bar = ppq * 4u;

        auto range = range_for(section);

        for (int bar = 0; bar < bars; ++bar) {
            const auto& chord_str = chord_progression[static_cast<std::size_t>(bar % chord_progression.size())];
            auto parsed = parse_chord(chord_str);
            std::uint32_t bar_start = static_cast<std::uint32_t>(bar) * ticks_per_bar;
            std::uint32_t duration = ticks_per_bar - 20;

            if (section == StringsSection::full_ensemble) {
                // Distribute chord across violin, viola, cello, contrabass.
                auto vln = voice_in_range(parsed.root_pc, parsed.quality, {55, 84, 0}, rng);
                auto vla = voice_in_range(parsed.root_pc, parsed.quality, {48, 76, 1}, rng);
                auto cel = voice_in_range(parsed.root_pc, parsed.quality, {36, 60, 2}, rng);
                auto cb  = voice_in_range(parsed.root_pc, parsed.quality, {28, 48, 3}, rng);

                auto emit = [&](const std::vector<int>& notes, std::uint8_t ch, std::uint8_t vel) {
                    for (int n : notes) {
                        out.push_back(make_note(bar_start, bar_start + duration,
                                                static_cast<std::uint8_t>(n), vel, ch));
                    }
                };

                std::uint8_t vel_vln = static_cast<std::uint8_t>(60 + static_cast<int>(rng() % 16));
                std::uint8_t vel_vla = static_cast<std::uint8_t>(60 + static_cast<int>(rng() % 16));
                std::uint8_t vel_cel = static_cast<std::uint8_t>(65 + static_cast<int>(rng() % 16));
                std::uint8_t vel_cb  = static_cast<std::uint8_t>(70 + static_cast<int>(rng() % 16));

                emit(vln, 0, vel_vln);
                emit(vla, 1, vel_vla);
                emit(cel, 2, vel_cel);
                emit(cb,  3, vel_cb);
            } else {
                // Single section: generate close voicing.
                auto notes = voice_in_range(parsed.root_pc, parsed.quality, range, rng);
                std::uint8_t vel_base = static_cast<std::uint8_t>(60 + static_cast<int>(rng() % 21));
                for (std::size_t i = 0; i < notes.size(); ++i) {
                    std::uint8_t vel = static_cast<std::uint8_t>(
                        std::min(127, vel_base + static_cast<int>(rng() % 11) - 5));
                    out.push_back(make_note(bar_start, bar_start + duration,
                                            static_cast<std::uint8_t>(notes[i]), vel, range.channel));
                }
            }
        }
        return out;
    }

    std::vector<MidiEvent> generate_countermelody(
        std::string_view key, std::string_view scale_name,
        const std::vector<std::string>& chord_progression,
        int bars, int bpm, StringsSection section, Seed seed) const override
    {
        (void)bpm;
        if (bars < 1 || chord_progression.empty()) return {};

        std::mt19937_64 rng(seed);
        std::vector<MidiEvent> out;
        const std::uint32_t ppq = static_cast<std::uint32_t>(kPpq);
        const std::uint32_t ticks_per_bar = ppq * 4u;
        const std::uint32_t tick_per_beat = ppq;

        auto range = range_for(section);
        int key_pc = parse_root_pc(key);

        // Get scale intervals for stepwise motion.
        std::vector<int> scale_pcs;
        if (scales_) {
            auto intervals = scales_->intervals_for(scale_name, key_pc);
            scale_pcs.assign(intervals.begin(), intervals.end());
        }
        if (scale_pcs.empty()) {
            for (int i = 0; i < 7; ++i) scale_pcs.push_back((key_pc + kMajor[i]) % 12);
        }

        // Build scale note pool in the upper half of the range.
        std::vector<int> scale_notes;
        for (int oct = range.min / 12; oct <= range.max / 12 + 1; ++oct) {
            int base = oct * 12;
            for (int pc : scale_pcs) {
                int n = base + (pc % 12);
                if (n >= range.min + 6 && n <= range.max) {
                    auto it = std::find(scale_notes.begin(), scale_notes.end(), n);
                    if (it == scale_notes.end()) scale_notes.push_back(n);
                }
            }
        }
        if (scale_notes.empty()) scale_notes = {60, 62, 64, 65, 67, 69, 71, 72};
        std::sort(scale_notes.begin(), scale_notes.end());

        // Pick a starting note.
        int current_note = scale_notes[static_cast<std::size_t>(rng() % scale_notes.size())];

        for (int bar = 0; bar < bars; ++bar) {
            std::uint32_t bar_start = static_cast<std::uint32_t>(bar) * ticks_per_bar;

            // Countermelody: mostly quarter and half notes, stepwise.
            for (int beat = 0; beat < 4; ++beat) {
                std::uint32_t beat_tick = bar_start + static_cast<std::uint32_t>(beat) * tick_per_beat;
                // Duration: half note on beats 0 and 2, quarters on others.
                std::uint32_t duration = (beat % 2 == 0) ? tick_per_beat * 2 - 10 : tick_per_beat - 10;
                std::uint8_t vel = static_cast<std::uint8_t>(65 + static_cast<int>(rng() % 21));

                out.push_back(make_note(beat_tick, beat_tick + duration,
                                       static_cast<std::uint8_t>(current_note), vel, range.channel));

                // Move stepwise to next note.
                int step = (rng() % 2 == 0) ? 1 : -1;
                int direction = (rng() % 3 == 0) ? -step : step; // occasional skip
                // Find next note in pool.
                int current_idx = -1;
                for (std::size_t i = 0; i < scale_notes.size(); ++i) {
                    if (scale_notes[i] == current_note) { current_idx = static_cast<int>(i); break; }
                }
                int next_idx = current_idx + direction;
                next_idx = std::max(0, std::min(static_cast<int>(scale_notes.size()) - 1, next_idx));
                current_note = scale_notes[static_cast<std::size_t>(next_idx)];
            }
        }
        return out;
    }

    std::vector<MidiEvent> generate_swells(
        std::string_view key, std::string_view scale,
        const std::vector<std::string>& chord_progression,
        int bars, int bpm, StringsSection section, Seed seed) const override
    {
        (void)key;
        (void)scale;
        (void)bpm;
        if (bars < 1 || chord_progression.empty()) return {};

        std::mt19937_64 rng(seed);
        std::vector<MidiEvent> out;
        const std::uint32_t ppq = static_cast<std::uint32_t>(kPpq);
        const std::uint32_t ticks_per_bar = ppq * 4u;

        auto range = range_for(section);

        for (int bar = 0; bar < bars; ++bar) {
            const auto& chord_str = chord_progression[static_cast<std::size_t>(bar % chord_progression.size())];
            auto parsed = parse_chord(chord_str);
            std::uint32_t bar_start = static_cast<std::uint32_t>(bar) * ticks_per_bar;
            std::uint32_t duration = ticks_per_bar - 5;

            if (section == StringsSection::full_ensemble) {
                auto vln = voice_in_range(parsed.root_pc, parsed.quality, {55, 84, 0}, rng);
                auto vla = voice_in_range(parsed.root_pc, parsed.quality, {48, 76, 1}, rng);
                auto cel = voice_in_range(parsed.root_pc, parsed.quality, {36, 60, 2}, rng);
                auto cb  = voice_in_range(parsed.root_pc, parsed.quality, {28, 48, 3}, rng);

                auto emit_swell = [&](const std::vector<int>& notes, std::uint8_t ch, int vel_start) {
                    for (int n : notes) {
                        // Velocity ramps over the bar: 60->100.
                        std::uint8_t start_vel = static_cast<std::uint8_t>(std::min(127, vel_start));
                        out.push_back(make_note(bar_start, bar_start + duration,
                                                static_cast<std::uint8_t>(n), start_vel, ch));
                        // Add a second event at midpoint with higher velocity for the swell effect.
                        std::uint8_t end_vel = static_cast<std::uint8_t>(std::min(127, vel_start + 30 + static_cast<int>(rng() % 11)));
                        out.push_back(make_note(bar_start + ticks_per_bar / 2, bar_start + duration,
                                                static_cast<std::uint8_t>(n), end_vel, ch));
                    }
                };

                emit_swell(vln, 0, 55 + static_cast<int>(rng() % 11));
                emit_swell(vla, 1, 55 + static_cast<int>(rng() % 11));
                emit_swell(cel, 2, 60 + static_cast<int>(rng() % 11));
                emit_swell(cb,  3, 65 + static_cast<int>(rng() % 11));
            } else {
                auto notes = voice_in_range(parsed.root_pc, parsed.quality, range, rng);
                for (int n : notes) {
                    std::uint8_t start_vel = static_cast<std::uint8_t>(55 + static_cast<int>(rng() % 11));
                    out.push_back(make_note(bar_start, bar_start + duration,
                                            static_cast<std::uint8_t>(n), start_vel, range.channel));
                    // Second layer for swell effect.
                    std::uint8_t end_vel = static_cast<std::uint8_t>(std::min(127, start_vel + 30 + static_cast<int>(rng() % 11)));
                    out.push_back(make_note(bar_start + ticks_per_bar / 2, bar_start + duration,
                                            static_cast<std::uint8_t>(n), end_vel, range.channel));
                }
            }
        }
        return out;
    }

private:
    std::shared_ptr<IScaleProvider> scales_;
    std::shared_ptr<IChordEngine>   chords_;

    static constexpr std::array<int, 7> kMajor = {0, 2, 4, 5, 7, 9, 11};
};

} // namespace

// ── Factory ─────────────────────────────────────────────────────────
std::unique_ptr<IStringsEngine> make_strings_engine(
    std::shared_ptr<IScaleProvider> scales,
    std::shared_ptr<IChordEngine> chords)
{
    return std::make_unique<StringsEngine>(std::move(scales), std::move(chords));
}

} // namespace aimidi::theory
