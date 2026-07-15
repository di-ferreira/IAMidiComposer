// Implementation: OrchestrateWorkflow (W9)
//
// Transforms a piano idea (chords + melody) into a full orchestral arrangement
// by delegating to OrchestrationEngine, StringsEngine, BassEngine, and DrumEngine.
//
// Steps:
//   1. Analyze piano input — separate melody (highest register) from harmony.
//   2. Extract chord progression from harmony notes via pitch-class matching.
//   3. Build an orchestral ensemble matching the requested density.
//   4. Distribute melody + chords via OrchestrationEngine::orchestrate_full().
//   5. Generate string pad via StringsEngine (if density > sparse).
//   6. Generate bass line via BassEngine.
//   7. Generate drum groove via DrumEngine (if density > sparse).
//   8. Return all tracks labelled by instrument name.
//
// Determinism: same seed + same input → identical output.
#include <aimidi/theory/IOrchestrateWorkflow.hpp>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <array>
#include <map>
#include <cstdint>

namespace aimidi::theory {

namespace {

// ── Pitch-class helpers ──────────────────────────────────────────────
const std::unordered_map<std::string_view, int>& key_table() {
    static const std::unordered_map<std::string_view, int> k = {
        {"C",0},  {"C#",1}, {"Db",1}, {"D",2},  {"D#",3}, {"Eb",3},
        {"E",4},  {"F",5},  {"F#",6}, {"Gb",6}, {"G",7},  {"G#",8},
        {"Ab",8}, {"A",9},  {"A#",10},{"Bb",10},{"B",11},
    };
    return k;
}

int root_pc_from_key(std::string_view key) {
    const auto& k = key_table();
    auto it = k.find(key);
    return it != k.end() ? it->second : 0;
}

constexpr const char* kPcNames[12] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

// ── Chord quality definitions (intervals from root) ──────────────────
struct ChordTemplate {
    std::string_view quality;
    std::vector<int> intervals; // semitones from root, ascending
};

const ChordTemplate kTemplates[] = {
    {"maj",  {0, 4, 7}},
    {"min",  {0, 3, 7}},
    {"7",    {0, 4, 7, 10}},
    {"m7",   {0, 3, 7, 10}},
    {"maj7", {0, 4, 7, 11}},
    {"dim",  {0, 3, 6}},
    {"dim7", {0, 3, 6, 9}},
    {"m7b5", {0, 3, 6, 10}},
};

// Extract set of pitch classes from a collection of notes.
std::unordered_set<int> pitch_classes(const std::vector<MidiEvent>& events) {
    std::unordered_set<int> pcs;
    for (const auto& ev : events) {
        pcs.insert(static_cast<int>(ev.note) % 12);
    }
    return pcs;
}

// Detect a chord symbol from a set of pitch classes.
// Scans all possible roots 0..11 and template matches.
// Returns a string like "C", "Am", "G7".
std::string detect_chord(const std::unordered_set<int>& pcs) {
    if (pcs.empty()) return "C";

    // Try every possible root and template combination.
    // Score: each matching interval +1, each extra/missing note penalized.
    int best_root = 0;
    std::string_view best_quality = "maj";
    int best_score = -999;

    for (int root = 0; root < 12; ++root) {
        for (const auto& tmpl : kTemplates) {
            int score = 0;
            for (int i = 0; i < 12; ++i) {
                bool in_pcs   = (pcs.count(i) > 0);
                bool in_chord = false;
                for (int interv : tmpl.intervals) {
                    if ((root + interv) % 12 == i) {
                        in_chord = true;
                        break;
                    }
                }
                if (in_pcs && in_chord)       score += 2;  // match
                else if (!in_pcs && in_chord)  score -= 1;  // missing chord tone
                else if (in_pcs && !in_chord)  score -= 1;  // extra non-chord tone
            }
            if (score > best_score) {
                best_score = score;
                best_root = root;
                best_quality = tmpl.quality;
            }
        }
    }

    // Build chord symbol: root name + quality suffix (skip "maj").
    std::string sym(kPcNames[best_root]);
    if (best_quality != "maj") {
        sym.append(best_quality.data(), best_quality.size());
    }
    return sym;
}

// Build chord progression from harmony events per bar.
std::vector<std::string> build_chord_progression(
    const std::vector<MidiEvent>& harmony, int bars)
{
    if (harmony.empty() || bars < 1) return {"C"};

    const std::uint32_t ticks_per_bar = static_cast<std::uint32_t>(kPpq) * 4u;
    std::vector<std::string> progression;
    progression.reserve(static_cast<std::size_t>(bars));

    for (int bar = 0; bar < bars; ++bar) {
        std::uint32_t bar_start = static_cast<std::uint32_t>(bar) * ticks_per_bar;
        std::uint32_t bar_end   = bar_start + ticks_per_bar;

        std::vector<MidiEvent> bar_harmony;
        for (const auto& ev : harmony) {
            if (ev.tick_on >= bar_start && ev.tick_on < bar_end) {
                bar_harmony.push_back(ev);
            }
        }

        auto pcs = pitch_classes(bar_harmony);
        progression.push_back(detect_chord(pcs));
    }

    return progression;
}

// Separate piano events into melody (highest note per time slice) and harmony.
void separate_melody_harmony(
    const std::vector<MidiEvent>& piano,
    std::vector<MidiEvent>& melody,
    std::vector<MidiEvent>& harmony)
{
    if (piano.empty()) return;

    // Group events by tick_on.
    std::map<std::uint32_t, std::vector<MidiEvent>> groups;
    for (const auto& ev : piano) {
        groups[ev.tick_on].push_back(ev);
    }

    for (const auto& [tick, events] : groups) {
        if (events.empty()) continue;

        // Find the highest-pitched note.
        auto highest = events.begin();
        for (auto it = events.begin(); it != events.end(); ++it) {
            if (it->note > highest->note) {
                highest = it;
            }
        }

        // Highest note → melody; all others → harmony.
        melody.push_back(*highest);
        for (auto it = events.begin(); it != events.end(); ++it) {
            if (it != highest) {
                harmony.push_back(*it);
            }
        }
    }
}

// Build a ChordRequest from chord_progression strings.
ChordRequest build_chord_request(
    const std::vector<std::string>& chord_progression,
    Seed seed)
{
    const std::uint32_t ppq = static_cast<std::uint32_t>(kPpq);
    const std::uint32_t ticks_per_bar = ppq * 4u;

    ChordRequest req;
    req.seed = seed;
    req.ppq = ppq;
    req.chords.reserve(chord_progression.size());

    for (std::size_t i = 0; i < chord_progression.size(); ++i) {
        const auto& sym = chord_progression[i];
        std::string root(1, sym[0]);
        std::size_t pos = 1;
        if (pos < sym.size() && (sym[pos] == '#' || sym[pos] == 'b')) {
            root += sym[pos];
            ++pos;
        }
        std::string qual(sym.substr(pos));
        if (qual.empty() || qual == "M") qual = "maj";

        ChordSpec cs;
        cs.start_tick     = static_cast<std::uint32_t>(i) * ticks_per_bar;
        cs.duration_ticks = ticks_per_bar - 20;
        cs.root_pc        = root_pc_from_key(root);
        cs.quality        = std::move(qual);
        cs.channel        = 0;
        req.chords.push_back(std::move(cs));
    }

    return req;
}

// ── Ensemble builder ─────────────────────────────────────────────────
std::vector<OrchestralSection> build_ensemble(
    OrchestrateDensity density, std::mt19937_64& rng)
{
    (void)rng;
    std::vector<OrchestralSection> sections;

    if (density == OrchestrateDensity::sparse) {
        // String quartet + bass
        OrchestralSection strings;
        strings.name = "Strings";
        strings.instruments = {
            {"Violin I",  40, 0, 55, 84, OrchestralRole::melody,        0},
            {"Violin II", 40, 1, 55, 84, OrchestralRole::harmony,       2},
            {"Viola",     41, 2, 48, 76, OrchestralRole::harmony,       3},
            {"Cello",     42, 3, 36, 60, OrchestralRole::bass,          4},
        };
        sections.push_back(std::move(strings));
        OrchestralSection rhythm;
        rhythm.name = "Rhythm";
        // Sparse: no drums, no guitar
        sections.push_back(std::move(rhythm));
    }
    else if (density == OrchestrateDensity::medium) {
        // String quartet + guitar + bass + piano
        OrchestralSection strings;
        strings.name = "Strings";
        strings.instruments = {
            {"Violin I",  40, 0, 55, 84, OrchestralRole::melody,        0},
            {"Violin II", 40, 1, 55, 84, OrchestralRole::harmony,       2},
            {"Viola",     41, 2, 48, 76, OrchestralRole::harmony,       3},
            {"Cello",     42, 3, 36, 60, OrchestralRole::bass,          4},
        };
        sections.push_back(std::move(strings));
        OrchestralSection rhythm;
        rhythm.name = "Rhythm";
        rhythm.instruments = {
            {"Contrabass", 44, 4, 28, 48, OrchestralRole::bass,   5},
            {"Guitar",     27, 5, 40, 76, OrchestralRole::harmony, 3},
            {"Drums",       1, 9, 35, 81, OrchestralRole::rhythm,  6},
        };
        sections.push_back(std::move(rhythm));
    }
    else { // full
        // Full orchestra: strings, woodwinds, brass, percussion, rhythm
        {
            OrchestralSection sec;
            sec.name = "Strings";
            sec.instruments = {
                {"Violin I",   40, 0, 55, 84, OrchestralRole::melody,        0},
                {"Violin II",  40, 1, 55, 84, OrchestralRole::countermelody, 1},
                {"Viola",      41, 2, 48, 76, OrchestralRole::harmony,       3},
                {"Cello",      42, 3, 36, 60, OrchestralRole::harmony,       4},
                {"Contrabass", 44, 4, 28, 48, OrchestralRole::bass,          5},
            };
            sections.push_back(std::move(sec));
        }
        {
            OrchestralSection sec;
            sec.name = "Woodwinds";
            sec.instruments = {
                {"Flute",    73, 5, 60, 96, OrchestralRole::countermelody, 2},
                {"Oboe",     68, 6, 58, 87, OrchestralRole::fill,          6},
                {"Clarinet", 71, 7, 50, 84, OrchestralRole::fill,          7},
            };
            sections.push_back(std::move(sec));
        }
        {
            OrchestralSection sec;
            sec.name = "Brass";
            sec.instruments = {
                {"Horn",    60, 8,  41, 77, OrchestralRole::harmony, 4},
                {"Trumpet", 56, 9,  58, 84, OrchestralRole::harmony, 5},
            };
            sections.push_back(std::move(sec));
        }
        {
            OrchestralSection sec;
            sec.name = "Rhythm";
            sec.instruments = {
                {"Guitar", 27, 10, 40, 76, OrchestralRole::harmony, 3},
                {"Drums",   1,  9, 35, 81, OrchestralRole::rhythm,  6},
            };
            sections.push_back(std::move(sec));
        }
    }

    return sections;
}

} // namespace

// ── OrchestrateWorkflow implementation ───────────────────────────────
class OrchestrateWorkflow final : public IOrchestrateWorkflow {
public:
    OrchestrateWorkflow(std::shared_ptr<IOrchestrationEngine> orch,
                        std::shared_ptr<IStringsEngine> strings,
                        std::shared_ptr<IGuitarEngine> guitar,
                        std::shared_ptr<IBassEngine> bass,
                        std::shared_ptr<IDrumEngine> drums)
        : orch_(std::move(orch))
        , strings_(std::move(strings))
        , guitar_(std::move(guitar))
        , bass_(std::move(bass))
        , drums_(std::move(drums))
    {}

    std::vector<std::pair<std::string, std::vector<MidiEvent>>> orchestrate(
        const OrchestrateRequest& req) const override
    {
        std::vector<std::pair<std::string, std::vector<MidiEvent>>> result;
        if (req.piano_input.empty() || req.bars < 1) {
            return result;
        }

        // 1. Separate melody from harmony.
        std::vector<MidiEvent> melody, harmony;
        separate_melody_harmony(req.piano_input, melody, harmony);

        if (melody.empty() && harmony.empty()) {
            return result;
        }

        // 2. Extract chord progression from harmony notes.
        auto chord_progression = build_chord_progression(harmony, req.bars);

        // 3. Build ensemble.
        std::mt19937_64 rng(req.seed);
        auto ensemble = build_ensemble(req.density, rng);

        // 4. Distribute via OrchestrationEngine.
        auto orch_tracks = orch_->orchestrate_full(
            melody, chord_progression, ensemble, req.bars, req.bpm, req.seed);

        // Collect into result map.
        std::unordered_map<std::string, std::vector<MidiEvent>> track_map;
        for (auto& [name, events] : orch_tracks) {
            track_map[std::move(name)] = std::move(events);
        }

        // 5. Generate strings pad (if density > sparse).
        if (req.density != OrchestrateDensity::sparse && strings_) {
            auto string_pad = strings_->generate_pad(
                req.key, req.scale, chord_progression,
                req.bars, req.bpm, StringsSection::full_ensemble,
                req.seed + 1);

            if (!string_pad.empty()) {
                track_map["Strings Pad"] = std::move(string_pad);
            }
        }

        // 6. Generate guitar comping (if density == full).
        if (req.density == OrchestrateDensity::full && guitar_) {
            auto guitar_chords = guitar_->generate_chords(
                req.key, req.scale, chord_progression,
                req.bars, req.bpm, GuitarStyle::arpeggio,
                req.seed + 2);

            if (!guitar_chords.empty()) {
                track_map["Guitar"] = std::move(guitar_chords);
            }
        }

        // 7. Generate bass line.
        if (bass_ && !chord_progression.empty()) {
            auto bass_req = build_chord_request(chord_progression, req.seed + 3);
            BassStyle bstyle;
            bstyle.seed            = static_cast<int>(req.seed + 3);
            bstyle.use_octave_jump = true;

            auto bass_events = bass_->generate(bass_req, bstyle);
            if (!bass_events.empty()) {
                track_map["Bass"] = std::move(bass_events);
            }
        }

        // 8. Generate drums (if density > sparse).
        if (req.density != OrchestrateDensity::sparse && drums_ && !chord_progression.empty()) {
            auto drum_req = build_chord_request(chord_progression, req.seed + 4);
            DrumStyle dstyle;
            dstyle.seed        = static_cast<int>(req.seed + 4);
            dstyle.use_ghosts  = true;
            dstyle.use_open_hh = (req.density == OrchestrateDensity::full);

            auto drum_events = drums_->generate(drum_req, dstyle);
            if (!drum_events.empty()) {
                track_map["Drums"] = std::move(drum_events);
            }
        }

        // 9. Convert map → vector ordered by priority.
        struct TrackOrder {
            std::string name;
            int priority;
        };
        std::vector<TrackOrder> order = {
            {"Violin I",      0},
            {"Violin II",     1},
            {"Flute",         2},
            {"Viola",         3},
            {"Cello",         4},
            {"Contrabass",    5},
            {"Strings Pad",   6},
            {"Guitar",        7},
            {"Bass",          8},
            {"Drums",         9},
            {"Horn",         10},
            {"Trumpet",      11},
            {"Oboe",         12},
            {"Clarinet",     13},
            {"Piano",        14},
        };

        for (const auto& tord : order) {
            auto it = track_map.find(tord.name);
            if (it != track_map.end() && !it->second.empty()) {
                result.emplace_back(std::move(it->first), std::move(it->second));
            }
        }

        // Append any remaining tracks not in the priority list.
        for (auto& [name, events] : track_map) {
            if (!events.empty()) {
                bool already = false;
                for (const auto& tord : order) {
                    if (tord.name == name) { already = true; break; }
                }
                if (!already) {
                    result.emplace_back(std::move(name), std::move(events));
                }
            }
        }

        return result;
    }

private:
    std::shared_ptr<IOrchestrationEngine> orch_;
    std::shared_ptr<IStringsEngine> strings_;
    std::shared_ptr<IGuitarEngine> guitar_;
    std::shared_ptr<IBassEngine> bass_;
    std::shared_ptr<IDrumEngine> drums_;
};

// ── Factory ──────────────────────────────────────────────────────────
std::unique_ptr<IOrchestrateWorkflow> make_orchestrate_workflow(
    std::shared_ptr<IOrchestrationEngine> orch,
    std::shared_ptr<IStringsEngine> strings,
    std::shared_ptr<IGuitarEngine> guitar,
    std::shared_ptr<IBassEngine> bass,
    std::shared_ptr<IDrumEngine> drums)
{
    return std::make_unique<OrchestrateWorkflow>(
        std::move(orch), std::move(strings),
        std::move(guitar), std::move(bass), std::move(drums));
}

} // namespace aimidi::theory
