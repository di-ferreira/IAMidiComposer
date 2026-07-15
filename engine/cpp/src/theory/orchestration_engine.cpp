// Implementation: OrchestrationEngine
//
// Distributes musical material (chords, melody) across an ensemble of
// instruments respecting register, doubling, balance, and texture.
//
// Voice distribution algorithm:
//   1. Sort instruments by role priority (melody > countermelody > harmony >
//      bass > pad > rhythm > fill).
//   2. Assign melody to the highest-priority melody instrument.
//   3. Distribute chord tones across harmony instruments using close voicing
//      for small ensembles and open voicing for large ensembles.
//   4. Assign the bass line to the bass instrument (root + fifth).
//   5. Fill remaining texture with pad / rhythm instruments.
//
// Register management:
//   Each instrument plays only within its defined [min_note, max_note] range.
//   Chord notes are transposed into the instrument's comfortable register.
//
// Doubling:
//   Unison — same notes across instruments (power).
//   Octave — same notes an octave apart (depth).
//   Third  — parallel thirds (sweetness).
//
// Determinism: same seed → identical output (std::mt19937_64).
#include <aimidi/theory/IOrchestrationEngine.hpp>
#include <random>
#include <algorithm>
#include <array>
#include <unordered_map>
#include <cstdint>

namespace aimidi::theory {

namespace {

// ── Pitch-class lookup ────────────────────────────────────────────────
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
    int pc = parse_root_pc(root);
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

// ── Chord-tone interval tables ───────────────────────────────────────
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

// ── Role priority (lower = higher priority) ───────────────────────────
int role_priority(OrchestralRole r) {
    switch (r) {
        case OrchestralRole::melody:        return 0;
        case OrchestralRole::countermelody: return 1;
        case OrchestralRole::harmony:       return 2;
        case OrchestralRole::bass:          return 3;
        case OrchestralRole::pad:           return 4;
        case OrchestralRole::rhythm:        return 5;
        case OrchestralRole::fill:          return 6;
    }
    return 7;
}

// Sort instruments: role priority first, then name for determinism.
void sort_instruments(std::vector<OrchestralInstrument>& insts) {
    std::stable_sort(insts.begin(), insts.end(),
        [](const OrchestralInstrument& a, const OrchestralInstrument& b) {
            int pa = role_priority(a.role);
            int pb = role_priority(b.role);
            if (pa != pb) return pa < pb;
            return a.name < b.name;
        });
}

// Collect all instruments from sections into a single flat list.
std::vector<OrchestralInstrument> flatten_ensemble(
    const std::vector<OrchestralSection>& ensemble)
{
    std::vector<OrchestralInstrument> all;
    for (const auto& section : ensemble) {
        for (const auto& inst : section.instruments) {
            all.push_back(inst);
        }
    }
    sort_instruments(all);
    return all;
}

// Pick the best note from `pool` for an instrument with range [min_note, max_note].
// Prefers notes in the middle of the instrument's range (the sweet spot).
int pick_note_in_range(const std::vector<int>& pool, int min_note, int max_note,
                       int chord_index)
{
    // First pass: find all pool notes in range.
    std::vector<int> candidates;
    for (int n : pool) {
        if (n >= min_note && n <= max_note) {
            candidates.push_back(n);
        }
    }
    if (candidates.empty()) {
        // Transpose the closest pool note into range.
        int fallback = pool.empty() ? 60 : pool[0];
        while (fallback < min_note) fallback += 12;
        while (fallback > max_note) fallback -= 12;
        if (fallback < min_note || fallback > max_note) {
            fallback = (min_note + max_note) / 2;
        }
        return fallback;
    }

    // Pick distributed by chord_index for variety across instruments.
    int idx = chord_index % static_cast<int>(candidates.size());
    return candidates[static_cast<std::size_t>(idx)];
}

// Distribute chord tones across a set of harmony instruments.
void assign_chord_to_harmony(
    const ParsedChord& chord,
    const std::vector<OrchestralInstrument>& targets,
    std::unordered_map<std::string, std::vector<MidiEvent>>& out,
    std::uint32_t bar_start, std::uint32_t duration,
    std::uint8_t base_vel, std::mt19937_64& rng)
{
    if (targets.empty()) return;

    // Build a pool of chord tones spanning the ensemble's range.
    int min_register = 127, max_register = 0;
    for (const auto& inst : targets) {
        if (inst.min_note < min_register) min_register = inst.min_note;
        if (inst.max_note > max_register) max_register = inst.max_note;
    }
    int center = (min_register + max_register) / 2;

    auto offsets = offsets_for(chord.quality);
    std::vector<int> pool;
    int pc = chord.root_pc % 12;
    for (int oct = -1; oct <= 2; ++oct) {
        int base_oct = (center / 12) + oct;
        for (int off : offsets) {
            int n = base_oct * 12 + ((pc + off) % 12);
            if (n >= 0 && n <= 127) pool.push_back(n);
        }
    }
    std::sort(pool.begin(), pool.end());
    pool.erase(std::unique(pool.begin(), pool.end()), pool.end());

    if (pool.empty()) {
        pool = {60, 64, 67};
    }

    // Assign notes to each instrument.
    int inst_idx = 0;
    for (const auto& inst : targets) {
        int note = pick_note_in_range(pool, inst.min_note, inst.max_note, inst_idx);
        MidiEvent e{};
        e.tick_on = bar_start;
        e.tick_off = bar_start + duration;
        e.channel = static_cast<std::uint8_t>(inst.channel);
        e.note = static_cast<std::uint8_t>(note);
        e.velocity = static_cast<std::uint8_t>(
            std::min(127, base_vel + static_cast<int>(rng() % 21) - 10));
        e.articulation = 0u;
        out[inst.name].push_back(e);

        // Remove assigned note to avoid duplicates (close voicing).
        auto it = std::find(pool.begin(), pool.end(), note);
        if (it != pool.end()) pool.erase(it);

        ++inst_idx;
    }
}

// Assign bass notes (root + fifth, cycling pattern) to bass instruments.
void assign_bass(
    const ParsedChord& chord,
    const std::vector<OrchestralInstrument>& bass_insts,
    std::unordered_map<std::string, std::vector<MidiEvent>>& out,
    std::uint32_t bar_start, std::uint32_t beat_ticks,
    std::mt19937_64& rng)
{
    for (const auto& inst : bass_insts) {
        int pc = chord.root_pc % 12;
        // Root on beat 1, fifth on beat 3.
        int root = (inst.min_note / 12) * 12 + pc;
        while (root < inst.min_note + 6) root += 12;
        while (root > inst.min_note + 18) root -= 12;
        if (root < inst.min_note) root += 12;
        if (root > inst.max_note) root -= 12;

        int fifth = root + 7;
        if (fifth > inst.max_note) fifth -= 12;
        if (fifth < inst.min_note) fifth += 12;

        std::uint8_t vel = static_cast<std::uint8_t>(70 + static_cast<int>(rng() % 21));

        MidiEvent e1{};
        e1.tick_on = bar_start;
        e1.tick_off = bar_start + beat_ticks * 2 - 10;
        e1.channel = static_cast<std::uint8_t>(inst.channel);
        e1.note = static_cast<std::uint8_t>(root);
        e1.velocity = vel;
        e1.articulation = 0u;
        out[inst.name].push_back(e1);

        MidiEvent e2{};
        e2.tick_on = bar_start + beat_ticks * 2;
        e2.tick_off = bar_start + beat_ticks * 4 - 10;
        e2.channel = static_cast<std::uint8_t>(inst.channel);
        e2.note = static_cast<std::uint8_t>(fifth);
        e2.velocity = static_cast<std::uint8_t>(vel - 5);
        e2.articulation = 0u;
        out[inst.name].push_back(e2);
    }
}

// Assign sustained pad notes across pad instruments.
void assign_pad(
    const ParsedChord& chord,
    const std::vector<OrchestralInstrument>& pad_insts,
    std::unordered_map<std::string, std::vector<MidiEvent>>& out,
    std::uint32_t bar_start, std::uint32_t duration,
    std::mt19937_64& rng)
{
    if (pad_insts.empty()) return;

    auto offsets = offsets_for(chord.quality);
    int pc = chord.root_pc % 12;

    for (const auto& inst : pad_insts) {
        int center = (inst.min_note + inst.max_note) / 2;
        int base_oct = center / 12;
        std::uint8_t vel = static_cast<std::uint8_t>(50 + static_cast<int>(rng() % 16));

        for (int off : offsets) {
            int n = base_oct * 12 + ((pc + off) % 12);
            if (n < inst.min_note) n += 12;
            if (n > inst.max_note) n -= 12;
            if (n < inst.min_note || n > inst.max_note) continue;

            MidiEvent e{};
            e.tick_on = bar_start;
            e.tick_off = bar_start + duration;
            e.channel = static_cast<std::uint8_t>(inst.channel);
            e.note = static_cast<std::uint8_t>(n);
            e.velocity = vel;
            e.articulation = 0u;
            out[inst.name].push_back(e);
        }
    }
}

// Group instruments by role.
void group_by_role(const std::vector<OrchestralInstrument>& insts,
                   std::vector<OrchestralInstrument>& melody_insts,
                   std::vector<OrchestralInstrument>& countermelody_insts,
                   std::vector<OrchestralInstrument>& harmony_insts,
                   std::vector<OrchestralInstrument>& bass_insts,
                   std::vector<OrchestralInstrument>& pad_insts,
                   std::vector<OrchestralInstrument>& rhythm_insts,
                   std::vector<OrchestralInstrument>& fill_insts)
{
    for (const auto& inst : insts) {
        switch (inst.role) {
            case OrchestralRole::melody:        melody_insts.push_back(inst); break;
            case OrchestralRole::countermelody: countermelody_insts.push_back(inst); break;
            case OrchestralRole::harmony:       harmony_insts.push_back(inst); break;
            case OrchestralRole::bass:          bass_insts.push_back(inst); break;
            case OrchestralRole::pad:           pad_insts.push_back(inst); break;
            case OrchestralRole::rhythm:        rhythm_insts.push_back(inst); break;
            case OrchestralRole::fill:          fill_insts.push_back(inst); break;
        }
    }
}

MidiEvent make_note(std::uint32_t tick_on, std::uint32_t tick_off,
                    std::uint8_t note, std::uint8_t velocity,
                    std::uint8_t channel)
{
    MidiEvent e{};
    e.tick_on      = tick_on;
    e.tick_off     = tick_off;
    e.channel      = channel;
    e.note         = note;
    e.velocity     = velocity;
    e.articulation = 0u;
    return e;
}

} // namespace

// ── Concrete OrchestrationEngine ─────────────────────────────────────
class OrchestrationEngine final : public IOrchestrationEngine {
public:
    OrchestrationEngine(std::shared_ptr<IScaleProvider> scales,
                        std::shared_ptr<IChordEngine> chords)
        : scales_(std::move(scales))
        , chords_(std::move(chords))
    {}

    std::vector<std::pair<std::string, std::vector<MidiEvent>>> orchestrate_chords(
        const std::vector<std::string>& chord_progression,
        const std::vector<OrchestralSection>& ensemble,
        int bars, int bpm, Seed seed) const override
    {
        (void)bpm;
        std::vector<std::pair<std::string, std::vector<MidiEvent>>> result;
        if (bars < 1 || chord_progression.empty() || ensemble.empty()) {
            return result;
        }

        std::mt19937_64 rng(seed);
        const std::uint32_t ppq = static_cast<std::uint32_t>(kPpq);
        const std::uint32_t ticks_per_bar = ppq * 4u;
        const std::uint32_t beat_ticks = ppq;

        auto all_insts = flatten_ensemble(ensemble);
        if (all_insts.empty()) return result;

        std::vector<OrchestralInstrument> mel, cmel, harm, bass, pad, rhythm, fill;
        group_by_role(all_insts, mel, cmel, harm, bass, pad, rhythm, fill);

        // Build melody from chord roots if we have melody instruments.
        std::vector<MidiEvent> melody_from_chords;
        if (!mel.empty()) {
            for (std::size_t i = 0; i < chord_progression.size(); ++i) {
                auto parsed = parse_chord(chord_progression[i]);
                int root = 60 + (parsed.root_pc % 12);
                std::uint32_t tick = static_cast<std::uint32_t>(i) * ticks_per_bar;
                melody_from_chords.push_back(
                    make_note(tick, tick + beat_ticks - 5,
                              static_cast<std::uint8_t>(root),
                              static_cast<std::uint8_t>(75 + rng() % 21), 0));
            }
        }

        // Per-bar output map.
        std::unordered_map<std::string, std::vector<MidiEvent>> output;

        for (int bar = 0; bar < bars; ++bar) {
            std::size_t chord_idx = static_cast<std::size_t>(bar % chord_progression.size());
            auto parsed = parse_chord(chord_progression[chord_idx]);
            std::uint32_t bar_start = static_cast<std::uint32_t>(bar) * ticks_per_bar;
            std::uint32_t dur = ticks_per_bar - 20;

            // Assign melody (from root-based melody).
            if (!mel.empty()) {
                // One melody note per bar.
                auto parsed_here = parsed;
                for (const auto& inst : mel) {
                    int root = (inst.min_note + inst.max_note) / 2;
                    int note = (root / 12) * 12 + (parsed_here.root_pc % 12);
                    while (note < inst.min_note + 6) note += 12;
                    while (note > inst.max_note - 6) note -= 12;
                    if (note < inst.min_note || note > inst.max_note) {
                        note = (inst.min_note + inst.max_note) / 2;
                    }
                    std::uint8_t vel = static_cast<std::uint8_t>(70 + rng() % 31);
                    output[inst.name].push_back(
                        make_note(bar_start, bar_start + beat_ticks - 5,
                                  static_cast<std::uint8_t>(note), vel,
                                  static_cast<std::uint8_t>(inst.channel)));
                }
            }

            // Countermelody: stepwise motion on chord tones.
            if (!cmel.empty()) {
                auto offsets = offsets_for(parsed.quality);
                for (const auto& inst : cmel) {
                    int center = (inst.min_note + inst.max_note) / 2;
                    int base_oct = center / 12;
                    for (int beat = 0; beat < 4; ++beat) {
                        int off = offsets[static_cast<std::size_t>(beat % offsets.size())];
                        int n = base_oct * 12 + ((parsed.root_pc + off) % 12);
                        if (n < inst.min_note) n += 12;
                        if (n > inst.max_note) n -= 12;
                        if (n < inst.min_note || n > inst.max_note) continue;
                        std::uint32_t bt = bar_start + static_cast<std::uint32_t>(beat) * beat_ticks;
                        std::uint8_t vel = static_cast<std::uint8_t>(60 + rng() % 21);
                        output[inst.name].push_back(
                            make_note(bt, bt + beat_ticks - 10,
                                      static_cast<std::uint8_t>(n), vel,
                                      static_cast<std::uint8_t>(inst.channel)));
                    }
                }
            }

            // Harmony.
            if (!harm.empty()) {
                assign_chord_to_harmony(parsed, harm, output, bar_start, dur,
                                        static_cast<std::uint8_t>(60), rng);
            }

            // Bass.
            if (!bass.empty()) {
                assign_bass(parsed, bass, output, bar_start, beat_ticks, rng);
            }

            // Pad.
            if (!pad.empty()) {
                assign_pad(parsed, pad, output, bar_start, dur, rng);
            }

            // Rhythm / fill: simple chord stabs on beats 2 and 4.
            if (!rhythm.empty() || !fill.empty()) {
                auto both = rhythm;
                both.insert(both.end(), fill.begin(), fill.end());
                auto offsets = offsets_for(parsed.quality);
                for (const auto& inst : both) {
                    int center = (inst.min_note + inst.max_note) / 2;
                    int base_oct = center / 12;
                    for (int beat = 1; beat < 4; beat += 2) {
                        for (int off : offsets) {
                            int n = base_oct * 12 + ((parsed.root_pc + off) % 12);
                            if (n < inst.min_note) n += 12;
                            if (n > inst.max_note) n -= 12;
                            if (n < inst.min_note || n > inst.max_note) continue;
                            std::uint32_t bt = bar_start + static_cast<std::uint32_t>(beat) * beat_ticks;
                            std::uint8_t vel = static_cast<std::uint8_t>(55 + rng() % 26);
                            output[inst.name].push_back(
                                make_note(bt, bt + beat_ticks / 2 - 5,
                                          static_cast<std::uint8_t>(n), vel,
                                          static_cast<std::uint8_t>(inst.channel)));
                        }
                    }
                }
            }
        }

        // Convert unordered_map to vector ordered by instrument priority.
        for (const auto& inst : all_insts) {
            auto it = output.find(inst.name);
            if (it != output.end() && !it->second.empty()) {
                result.emplace_back(it->first, std::move(it->second));
            }
        }

        return result;
    }

    std::vector<std::pair<std::string, std::vector<MidiEvent>>> orchestrate_full(
        const std::vector<MidiEvent>& melody,
        const std::vector<std::string>& chord_progression,
        const std::vector<OrchestralSection>& ensemble,
        int bars, int bpm, Seed seed) const override
    {
        (void)bpm;
        std::vector<std::pair<std::string, std::vector<MidiEvent>>> result;
        if (melody.empty() || chord_progression.empty() || ensemble.empty() || bars < 1) {
            return result;
        }

        std::mt19937_64 rng(seed);
        const std::uint32_t ppq = static_cast<std::uint32_t>(kPpq);
        const std::uint32_t ticks_per_bar = ppq * 4u;
        const std::uint32_t beat_ticks = ppq;

        auto all_insts = flatten_ensemble(ensemble);
        if (all_insts.empty()) return result;

        std::vector<OrchestralInstrument> mel, cmel, harm, bass, pad, rhythm, fill;
        group_by_role(all_insts, mel, cmel, harm, bass, pad, rhythm, fill);
        // Re-sort grouped vectors by priority within each role.
        auto sort_by_name = [](std::vector<OrchestralInstrument>& v) {
            std::stable_sort(v.begin(), v.end(),
                [](const OrchestralInstrument& a, const OrchestralInstrument& b) {
                    return a.name < b.name;
                });
        };
        sort_by_name(mel);
        sort_by_name(cmel);
        sort_by_name(harm);

        std::unordered_map<std::string, std::vector<MidiEvent>> output;

        // Assign melody to melody instruments (primary) and countermelody (secondary).
        // Transpose melody into each instrument's range.
        for (const auto& inst : mel) {
            for (const auto& note : melody) {
                int n = static_cast<int>(note.note);
                // Transpose into instrument range.
                while (n < inst.min_note + 6) n += 12;
                while (n > inst.max_note - 6) n -= 12;
                if (n < inst.min_note || n > inst.max_note) {
                    n = (inst.min_note + inst.max_note) / 2;
                }
                MidiEvent e = note;
                e.channel = static_cast<std::uint8_t>(inst.channel);
                e.note = static_cast<std::uint8_t>(n);
                output[inst.name].push_back(e);
            }
        }

        // Countermelody: harmonize melody at the third or sixth.
        if (!cmel.empty() && !melody.empty()) {
            for (const auto& inst : cmel) {
                for (const auto& note : melody) {
                    // Simple harmonization: add a third below or above.
                    int interval = 3; // minor third by default
                    int n = static_cast<int>(note.note) - interval;
                    // Adjust to chord context via scale.
                    while (n < inst.min_note + 6) n += 12;
                    while (n > inst.max_note - 6) n -= 12;
                    if (n < inst.min_note || n > inst.max_note) {
                        n = (inst.min_note + inst.max_note) / 2;
                    }
                    MidiEvent e = note;
                    e.channel = static_cast<std::uint8_t>(inst.channel);
                    e.note = static_cast<std::uint8_t>(n);
                    e.velocity = static_cast<std::uint8_t>(
                        std::min(127, note.velocity - 10));
                    output[inst.name].push_back(e);
                }
            }
        }

        // Find harmony, bass, pad, rhythm/fill instruments for chord backing.
        for (int bar = 0; bar < bars; ++bar) {
            std::size_t chord_idx = static_cast<std::size_t>(bar % chord_progression.size());
            auto parsed = parse_chord(chord_progression[chord_idx]);
            std::uint32_t bar_start = static_cast<std::uint32_t>(bar) * ticks_per_bar;
            std::uint32_t dur = ticks_per_bar - 20;

            if (!harm.empty()) {
                assign_chord_to_harmony(parsed, harm, output, bar_start, dur,
                                        static_cast<std::uint8_t>(55), rng);
            }

            if (!bass.empty()) {
                assign_bass(parsed, bass, output, bar_start, beat_ticks, rng);
            }

            if (!pad.empty()) {
                assign_pad(parsed, pad, output, bar_start, dur, rng);
            }

            // Rhythm/fill stabs.
            if (!rhythm.empty() || !fill.empty()) {
                auto both = rhythm;
                both.insert(both.end(), fill.begin(), fill.end());
                auto offsets = offsets_for(parsed.quality);
                for (const auto& inst : both) {
                    int center = (inst.min_note + inst.max_note) / 2;
                    int base_oct = center / 12;
                    for (int beat = 1; beat < 4; beat += 2) {
                        for (int off : offsets) {
                            int n = base_oct * 12 + ((parsed.root_pc + off) % 12);
                            if (n < inst.min_note) n += 12;
                            if (n > inst.max_note) n -= 12;
                            if (n < inst.min_note || n > inst.max_note) continue;
                            std::uint32_t bt = bar_start + static_cast<std::uint32_t>(beat) * beat_ticks;
                            std::uint8_t vel = static_cast<std::uint8_t>(55 + rng() % 21);
                            output[inst.name].push_back(
                                make_note(bt, bt + beat_ticks / 2 - 5,
                                          static_cast<std::uint8_t>(n), vel,
                                          static_cast<std::uint8_t>(inst.channel)));
                        }
                    }
                }
            }
        }

        for (const auto& inst : all_insts) {
            auto it = output.find(inst.name);
            if (it != output.end() && !it->second.empty()) {
                result.emplace_back(it->first, std::move(it->second));
            }
        }

        return result;
    }

    std::vector<OrchestralSection> suggest_ensemble(
        std::string_view genre, int instrument_count) const override
    {
        std::vector<OrchestralSection> ensemble;

        auto add_inst = [](OrchestralSection& sec, const std::string& name,
                           int gm, int ch, int min_n, int max_n,
                           OrchestralRole role, int prio)
        {
            sec.instruments.push_back({name, gm, ch, min_n, max_n, role, prio});
        };

        // String quartet: 2 violins, viola, cello
        auto string_quartet = [&]() {
            OrchestralSection sec;
            sec.name = "Strings";
            add_inst(sec, "Violin I",  40, 0, 55, 84, OrchestralRole::melody,        0);
            add_inst(sec, "Violin II", 40, 1, 55, 84, OrchestralRole::harmony,       2);
            add_inst(sec, "Viola",     41, 2, 48, 76, OrchestralRole::harmony,       3);
            add_inst(sec, "Cello",     42, 3, 36, 60, OrchestralRole::bass,          4);
            return sec;
        };

        // Jazz combo: piano, bass, drums, guitar, sax
        auto jazz_combo = [&]() {
            OrchestralSection sec;
            sec.name = "Jazz Combo";
            add_inst(sec, "Piano",   1,  0, 21, 108, OrchestralRole::harmony,  2);
            add_inst(sec, "Bass",    34, 1, 28, 48,  OrchestralRole::bass,     3);
            add_inst(sec, "Drums",   1,  9, 35, 81,  OrchestralRole::rhythm,   5);
            add_inst(sec, "Guitar",  27, 2, 40, 76,  OrchestralRole::harmony,  3);
            add_inst(sec, "Sax",     67, 3, 49, 84,  OrchestralRole::melody,   0);
            return sec;
        };

        // Rock band: guitar, bass, drums, keys
        auto rock_band = [&]() {
            OrchestralSection sec;
            sec.name = "Rock Band";
            add_inst(sec, "Guitar",  30, 0, 40, 76, OrchestralRole::melody,  0);
            add_inst(sec, "Bass",    34, 1, 28, 48, OrchestralRole::bass,    3);
            add_inst(sec, "Drums",   1,  9, 35, 81, OrchestralRole::rhythm,  5);
            add_inst(sec, "Keys",    5,  2, 36, 96, OrchestralRole::harmony, 2);
            return sec;
        };

        // Orchestra: strings, woodwinds, brass, percussion
        auto orchestra = [&]() {
            std::vector<OrchestralSection> sections;
            {
                OrchestralSection sec;
                sec.name = "Strings";
                add_inst(sec, "Violin I",   40, 0, 55, 84, OrchestralRole::melody,        0);
                add_inst(sec, "Violin II",  40, 1, 55, 84, OrchestralRole::countermelody, 2);
                add_inst(sec, "Viola",      41, 2, 48, 76, OrchestralRole::harmony,       3);
                add_inst(sec, "Cello",      42, 3, 36, 60, OrchestralRole::harmony,       4);
                add_inst(sec, "Contrabass", 44, 4, 28, 48, OrchestralRole::bass,          5);
                sections.push_back(std::move(sec));
            }
            {
                OrchestralSection sec;
                sec.name = "Woodwinds";
                add_inst(sec, "Flute",      73, 5, 60, 96, OrchestralRole::countermelody, 1);
                add_inst(sec, "Oboe",       68, 6, 58, 87, OrchestralRole::fill,          6);
                add_inst(sec, "Clarinet",   71, 7, 50, 84, OrchestralRole::fill,          7);
                add_inst(sec, "Bassoon",    70, 8, 34, 66, OrchestralRole::fill,          8);
                sections.push_back(std::move(sec));
            }
            {
                OrchestralSection sec;
                sec.name = "Brass";
                add_inst(sec, "Horn",       60, 9,  41, 77, OrchestralRole::harmony, 4);
                add_inst(sec, "Trumpet",    56, 10, 58, 84, OrchestralRole::harmony, 5);
                add_inst(sec, "Trombone",   57, 11, 36, 72, OrchestralRole::fill,     7);
                sections.push_back(std::move(sec));
            }
            {
                OrchestralSection sec;
                sec.name = "Percussion";
                add_inst(sec, "Timpani",    47, 12, 28, 55, OrchestralRole::rhythm, 6);
                sections.push_back(std::move(sec));
            }
            return sections;
        };

        // Pop band: piano, guitar, bass, drums, strings
        auto pop_band = [&]() {
            std::vector<OrchestralSection> sections;
            {
                OrchestralSection sec;
                sec.name = "Rhythm Section";
                add_inst(sec, "Piano",  1,  0, 21, 108, OrchestralRole::harmony, 2);
                add_inst(sec, "Guitar", 27, 1, 40, 76,  OrchestralRole::harmony, 3);
                add_inst(sec, "Bass",   34, 2, 28, 48,  OrchestralRole::bass,    4);
                add_inst(sec, "Drums",  1,  9, 35, 81,  OrchestralRole::rhythm,  5);
                sections.push_back(std::move(sec));
            }
            {
                OrchestralSection sec;
                sec.name = "Strings";
                add_inst(sec, "Violin I",  40, 3, 55, 84, OrchestralRole::countermelody, 1);
                add_inst(sec, "Cello",     42, 4, 36, 60, OrchestralRole::pad,           6);
                sections.push_back(std::move(sec));
            }
            return sections;
        };

        if (genre == "jazz" || genre == "jazz combo") {
            auto sec = jazz_combo();
            ensemble.push_back(std::move(sec));
        } else if (genre == "rock" || genre == "rock band") {
            auto sec = rock_band();
            ensemble.push_back(std::move(sec));
        } else if (genre == "orchestra" || genre == "orchestral") {
            ensemble = orchestra();
        } else if (genre == "pop" || genre == "pop band") {
            ensemble = pop_band();
        } else if (genre == "string quartet") {
            auto sec = string_quartet();
            ensemble.push_back(std::move(sec));
        } else {
            // Default: pop band.
            ensemble = pop_band();
        }

        // Limit instrument count if requested.
        if (instrument_count > 0 && static_cast<int>(total_instrument_count(ensemble)) > instrument_count) {
            ensemble = trim_ensemble(ensemble, instrument_count);
        }

        return ensemble;
    }

private:
    std::shared_ptr<IScaleProvider> scales_;
    std::shared_ptr<IChordEngine>   chords_;

    static int total_instrument_count(const std::vector<OrchestralSection>& ens) {
        int count = 0;
        for (const auto& sec : ens) count += static_cast<int>(sec.instruments.size());
        return count;
    }

    static std::vector<OrchestralSection> trim_ensemble(
        const std::vector<OrchestralSection>& ens, int max_instruments)
    {
        // Collect all instruments, sort by priority, keep top N.
        std::vector<OrchestralInstrument> all;
        for (const auto& sec : ens) {
            for (const auto& inst : sec.instruments) {
                all.push_back(inst);
            }
        }
        sort_instruments(all);
        if (static_cast<int>(all.size()) > max_instruments) {
            all.resize(static_cast<std::size_t>(max_instruments));
        }
        // Rebuild sections: put everything in one "Ensemble" section.
        OrchestralSection single;
        single.name = "Ensemble";
        single.instruments = std::move(all);
        return {single};
    }
};

// ── Factory ──────────────────────────────────────────────────────────
std::unique_ptr<IOrchestrationEngine> make_orchestration_engine(
    std::shared_ptr<IScaleProvider> scales,
    std::shared_ptr<IChordEngine> chords)
{
    return std::make_unique<OrchestrationEngine>(std::move(scales), std::move(chords));
}

} // namespace aimidi::theory
