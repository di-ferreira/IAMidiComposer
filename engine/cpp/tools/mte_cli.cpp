// mte_cli.cpp — CLI tool that chains all MTE engines and produces an SMF file.
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IHarmonyEngine.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <aimidi/theory/IBassEngine.hpp>
#include <aimidi/theory/IRhythmEngine.hpp>
#include <aimidi/theory/IDrumEngine.hpp>
#include <aimidi/theory/IPianoEngine.hpp>
#include <aimidi/theory/IHumanizationEngine.hpp>
#include <aimidi/theory/IMidiRenderer.hpp>
#include <aimidi/theory/Types.hpp>

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace {

// ---------------------------------------------------------------------------
// CLI argument parsing
// ---------------------------------------------------------------------------
struct Args {
    std::uint64_t seed   = 0;
    std::string   key;
    std::string   scale;
    int           bpm    = 120;
    int           bars   = 4;
    std::string   output;
};

void print_usage(const char* prog) {
    std::fprintf(stderr,
        "Usage: %s --seed <n> --key <key> --scale <scale>"
        " --bpm <bpm> --bars <n> --output <file.mid>\n",
        prog);
}

bool parse_args(int argc, char* argv[], Args& args) {
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--seed") == 0) {
            if (++i >= argc) return false;
            char* end = nullptr;
            args.seed = std::strtoull(argv[i], &end, 10);
            if (*end != '\0') return false;
        } else if (std::strcmp(argv[i], "--key") == 0) {
            if (++i >= argc) return false;
            args.key = argv[i];
        } else if (std::strcmp(argv[i], "--scale") == 0) {
            if (++i >= argc) return false;
            args.scale = argv[i];
        } else if (std::strcmp(argv[i], "--bpm") == 0) {
            if (++i >= argc) return false;
            args.bpm = std::atoi(argv[i]);
            if (args.bpm <= 0) return false;
        } else if (std::strcmp(argv[i], "--bars") == 0) {
            if (++i >= argc) return false;
            args.bars = std::atoi(argv[i]);
            if (args.bars <= 0) return false;
        } else if (std::strcmp(argv[i], "--output") == 0) {
            if (++i >= argc) return false;
            args.output = argv[i];
        } else {
            return false;
        }
    }
    return !args.key.empty() && !args.scale.empty() && !args.output.empty();
}

// ---------------------------------------------------------------------------
// Key name → root pitch-class (0..11) mapping
// ---------------------------------------------------------------------------
int key_to_root_pc(const std::string& key) {
    static const struct { const char* name; int pc; } kTable[] = {
        {"C", 0}, {"C#", 1}, {"Db", 1},
        {"D", 2}, {"D#", 3}, {"Eb", 3},
        {"E", 4},
        {"F", 5}, {"F#", 6}, {"Gb", 6},
        {"G", 7}, {"G#", 8}, {"Ab", 8},
        {"A", 9}, {"A#", 10}, {"Bb", 10},
        {"B", 11},
    };
    for (const auto& entry : kTable) {
        if (key == entry.name) return entry.pc;
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Chord detection from a group of MidiEvent notes
// ---------------------------------------------------------------------------
struct ChordDetection {
    bool        valid   = false;
    int         root_pc = 0;
    std::string quality = "maj";
};

ChordDetection detect_chord(const std::vector<aimidi::theory::MidiEvent>& events) {
    if (events.empty()) return {};

    std::set<int> pcs;
    for (const auto& e : events) {
        pcs.insert(static_cast<int>(e.note % 12));
    }

    static const struct { const char* name; std::set<int> intervals; } kTemplates[] = {
        {"maj",   {4, 7}},
        {"min",   {3, 7}},
        {"7",     {4, 7, 10}},
        {"m7",    {3, 7, 10}},
        {"maj7",  {4, 7, 11}},
    };

    int         best_root    = *pcs.begin();
    std::string best_quality = "maj";
    int         best_score   = -1;

    for (int root : pcs) {
        std::set<int> intervals;
        for (int pc : pcs) {
            if (pc == root) continue;
            intervals.insert((pc - root + 12) % 12);
        }

        for (const auto& tmpl : kTemplates) {
            bool match = true;
            for (int iv : tmpl.intervals) {
                if (intervals.find(iv) == intervals.end()) {
                    match = false;
                    break;
                }
            }
            if (match) {
                int score = static_cast<int>(tmpl.intervals.size());
                if (score > best_score) {
                    best_score   = score;
                    best_root    = root;
                    best_quality = tmpl.name;
                }
            }
        }
    }

    return {best_score >= 0, best_root, best_quality};
}

} // unnamed namespace

// ===========================================================================
int main(int argc, char* argv[]) {
    Args args;
    if (!parse_args(argc, argv, args)) {
        print_usage(argv[0]);
        return 1;
    }

    int root_pc = key_to_root_pc(args.key);
    if (root_pc < 0) {
        std::fprintf(stderr, "Error: unknown key '%s'\n", args.key.c_str());
        return 1;
    }
    (void)root_pc; // used implicitly through harmony engine

    // ---- a. ScaleProvider ------------------------------------------------
    auto scales = aimidi::theory::make_scale_provider();
    if (!scales->knows(args.scale)) {
        std::fprintf(stderr, "Error: unknown scale '%s'\n", args.scale.c_str());
        return 1;
    }

    // ---- b. ChordEngine --------------------------------------------------
    auto chord_engine = aimidi::theory::make_chord_engine();

    // ---- c. HarmonyEngine (DI) -------------------------------------------
    std::shared_ptr<aimidi::theory::IScaleProvider> shared_scales = std::move(scales);
    std::shared_ptr<aimidi::theory::IChordEngine>   shared_chords = std::move(chord_engine);
    auto harmony = aimidi::theory::make_harmony_engine(shared_scales, shared_chords);

    // ---- d. Generate harmony ---------------------------------------------
    aimidi::theory::HarmonyRequest harm_req{args.key, args.scale, args.bars, args.seed};
    auto harmony_events = harmony->generate(harm_req);

    // ---- e. Convert harmony events → ChordRequest (one chord per bar) ----
    std::map<std::uint32_t, std::vector<aimidi::theory::MidiEvent>> chord_groups;
    for (const auto& ev : harmony_events) {
        chord_groups[ev.tick_on].push_back(ev);
    }

    constexpr int kTicksPerBar = aimidi::theory::kPpq * 4;

    std::vector<std::uint32_t> tick_ons;
    tick_ons.reserve(chord_groups.size());
    for (const auto& [tick, _] : chord_groups) {
        tick_ons.push_back(tick);
    }

    std::vector<aimidi::theory::ChordSpec> chord_specs;
    chord_specs.reserve(tick_ons.size());
    for (std::size_t i = 0; i < tick_ons.size(); ++i) {
        auto detection = detect_chord(chord_groups[tick_ons[i]]);
        if (!detection.valid) {
            std::fprintf(stderr,
                "Warning: could not detect chord at tick %u, falling back to Cmaj\n",
                tick_ons[i]);
            detection = {true, 0, "maj"};
        }
        std::uint32_t duration = (i + 1 < tick_ons.size())
                                     ? tick_ons[i + 1] - tick_ons[i]
                                     : kTicksPerBar;
        chord_specs.push_back({tick_ons[i], duration,
                               detection.root_pc, detection.quality, 0});
    }

    aimidi::theory::ChordRequest chord_req{chord_specs, args.seed,
                                            aimidi::theory::kPpq};

    // ---- f. BassEngine ---------------------------------------------------
    auto bass = aimidi::theory::make_bass_engine();
    aimidi::theory::BassStyle bass_style{
        .seed = static_cast<int>(args.seed), .use_octave_jump = false};
    auto bass_events = bass->generate(chord_req, bass_style);

    // ---- g. RhythmEngine (eighth notes, no swing, no fill) ---------------
    auto rhythm = aimidi::theory::make_rhythm_engine();
    aimidi::theory::RhythmStyle rhythm_style{
        .resolution = aimidi::theory::RhythmResolution::eighth,
        .swing      = 0.0f,
        .seed       = args.seed};
    aimidi::theory::FillSpec no_fill{-1, 1};
    auto rhythm_events = rhythm->generate(chord_req, rhythm_style, no_fill);

    // ---- h. DrumEngine ---------------------------------------------------
    auto drums = aimidi::theory::make_drum_engine();
    aimidi::theory::DrumStyle drum_style{
        .seed = static_cast<int>(args.seed), .use_ghosts = true, .use_open_hh = false};
    auto drum_events = drums->generate(chord_req, drum_style);

    // ---- i. PianoEngine (block chords) -----------------------------------
    auto piano = aimidi::theory::make_piano_engine();
    aimidi::theory::PianoRequest piano_req{
        .chord_req     = chord_req,
        .style         = aimidi::theory::PianoStyle::block_chords,
        .pedal         = {},
        .bass_octave   = 3,
        .treble_octave = 4,
    };
    auto piano_events = piano->generate(piano_req);

    // ---- j. HumanizationEngine (timing_jitter=3, velocity_variation=5) ---
    auto human = aimidi::theory::make_humanization_engine();
    aimidi::theory::HumanizationParams human_params{
        .seed                = args.seed,
        .timing_jitter_ticks = 3,
        .velocity_variation  = 5,
        .groove_swing        = 0.0f,
        .snap_to_grid        = false,
        .grid_resolution     = 120,
    };
    human->apply(piano_events, human_params);
    human->apply(bass_events, human_params);

    // ---- k. Combine drums (RhythmEngine + DrumEngine) --------------------
    std::vector<aimidi::theory::MidiEvent> drums_combined;
    drums_combined.reserve(rhythm_events.size() + drum_events.size());
    drums_combined.insert(drums_combined.end(), rhythm_events.begin(),
                          rhythm_events.end());
    drums_combined.insert(drums_combined.end(), drum_events.begin(),
                          drum_events.end());
    std::sort(drums_combined.begin(), drums_combined.end(),
              [](const aimidi::theory::MidiEvent& a,
                 const aimidi::theory::MidiEvent& b) {
                  return a.tick_on < b.tick_on;
              });

    // ---- l. MidiRenderer → SmfComposition --------------------------------
    auto renderer = aimidi::theory::make_midi_renderer();
    aimidi::theory::SmfComposition comp;
    comp.ppq           = aimidi::theory::kPpq;
    comp.bpm           = args.bpm;
    comp.key           = args.key;
    comp.scale         = args.scale;
    comp.time_sig_num  = 4;
    comp.time_sig_den  = 4;
    comp.tracks        = {
        {"Piano", std::move(piano_events)},
        {"Bass",  std::move(bass_events)},
        {"Drums", std::move(drums_combined)},
    };

    // ---- m. Render to SMF bytes ------------------------------------------
    auto smf = renderer->render(comp);

    // ---- n. Write output file --------------------------------------------
    std::ofstream out(args.output, std::ios::binary);
    if (!out) {
        std::fprintf(stderr, "Error: could not open '%s' for writing\n",
                     args.output.c_str());
        return 1;
    }
    out.write(reinterpret_cast<const char*>(smf.data()),
              static_cast<std::streamsize>(smf.size()));
    if (!out) {
        std::fprintf(stderr, "Error: write failed for '%s'\n",
                     args.output.c_str());
        return 1;
    }

    std::fprintf(stdout, "Wrote %zu bytes to %s\n", smf.size(),
                 args.output.c_str());
    return 0;
}
