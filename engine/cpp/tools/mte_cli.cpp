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
#include <aimidi/theory/IPromptInterpreter.hpp>
#include <aimidi/theory/IBlueprintGenerator.hpp>
#include <aimidi/theory/ITimelinePlanner.hpp>
#include <aimidi/theory/IArrangementPlanner.hpp>
#include <aimidi/theory/IInstrumentMapper.hpp>
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
    std::string   prompt;
    bool          verbose = false;
};

void print_usage(const char* prog) {
    std::fprintf(stderr,
        "Usage: %s [options]\n"
        "\nOptions:\n"
        "  --prompt <text>    Natural language prompt\n"
        "                     (e.g. \"upbeat pop song in C major\")\n"
        "  --seed <n>         Random seed (default: 0 = auto from prompt)\n"
        "  --key <key>        Root key (default: from prompt or C)\n"
        "  --scale <scale>    Scale (default: from prompt or major)\n"
        "  --bpm <n>          BPM (default: from prompt or 120)\n"
        "  --bars <n>         Number of bars (default: from prompt or 4)\n"
        "  --output <file>    Output SMF file path (required)\n"
        "  --verbose          Print blueprint/timeline/arrangement info\n",
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
        } else if (std::strcmp(argv[i], "--prompt") == 0) {
            if (++i >= argc) return false;
            args.prompt = argv[i];
        } else if (std::strcmp(argv[i], "--verbose") == 0) {
            args.verbose = true;
        } else {
            return false;
        }
    }
    if (args.output.empty()) return false;
    if (args.prompt.empty() && (args.key.empty() || args.scale.empty()))
        return false;
    return true;
}

// ---------------------------------------------------------------------------
// Key name -> root pitch-class (0..11) mapping
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
    using namespace aimidi::theory;

    Args args;
    if (!parse_args(argc, argv, args)) {
        print_usage(argv[0]);
        return 1;
    }

    // Resolved parameters (from prompt or CLI args)
    std::string resolved_key;
    std::string resolved_scale;
    int         resolved_bpm  = 120;
    int         resolved_bars = 4;
    Seed        resolved_seed = args.seed;

    // MI pipeline state (only when --prompt is used)
    MusicBlueprint blueprint;
    Timeline timeline;
    Arrangement arrangement;
    std::unique_ptr<IInstrumentMapper> instrument_mapper;

    if (!args.prompt.empty()) {
        // ===============================================================
        // Musical Intelligence pipeline
        // ===============================================================

        // 1. Parse prompt via IPromptInterpreter
        auto interpreter = make_prompt_interpreter();
        blueprint = interpreter->interpret(args.prompt, resolved_seed);

        // Auto-seed from prompt hash when seed is 0 (not explicitly set)
        if (resolved_seed == 0) {
            resolved_seed = blueprint.seed;
        }

        // 2. Finalize via IBlueprintGenerator
        auto bp_gen = make_blueprint_generator();
        blueprint = bp_gen->generate(blueprint);

        // 3. Plan timeline via ITimelinePlanner
        auto tl_planner = make_timeline_planner();
        timeline = tl_planner->plan(blueprint);

        // 4. Plan arrangement via IArrangementPlanner
        auto arr_planner = make_arrangement_planner();
        arrangement = arr_planner->plan(blueprint, timeline);

        // 5. Create instrument mapper
        instrument_mapper = make_instrument_mapper();

        // Resolve effective parameters from blueprint + CLI overrides
        resolved_key   = blueprint.root_key;
        resolved_scale = blueprint.scale;
        resolved_bpm   = blueprint.bpm;
        resolved_bars  = timeline.total_bars;

        // CLI overrides take precedence
        if (!args.key.empty())   resolved_key   = args.key;
        if (!args.scale.empty()) resolved_scale = args.scale;
        if (args.bpm != 120)     resolved_bpm   = args.bpm;
        if (args.bars != 4)      resolved_bars  = args.bars;

        // ---- Verbose output (stderr) ---------------------------------
        if (args.verbose) {
            std::fprintf(stderr, "=== Blueprint ===\n");
            std::fprintf(stderr, "  key:       %s\n", resolved_key.c_str());
            std::fprintf(stderr, "  scale:     %s\n", resolved_scale.c_str());
            std::fprintf(stderr, "  bpm:       %d\n", resolved_bpm);
            std::fprintf(stderr, "  seed:      %llu\n",
                         static_cast<unsigned long long>(resolved_seed));
            std::fprintf(stderr, "  genres:    ");
            for (size_t gi = 0; gi < blueprint.genres.size(); ++gi) {
                if (gi > 0) std::fprintf(stderr, ", ");
                std::fprintf(stderr, "%s", blueprint.genres[gi].c_str());
            }
            std::fprintf(stderr, "\n");
            std::fprintf(stderr, "  emotion:   valence=%.2f  arousal=%.2f  dominance=%.2f\n",
                         static_cast<double>(blueprint.emotion.valence),
                         static_cast<double>(blueprint.emotion.arousal),
                         static_cast<double>(blueprint.emotion.dominance));
            std::fprintf(stderr, "  energy:    %d\n",
                         static_cast<int>(blueprint.energy));
            std::fprintf(stderr, "  sections:\n");
            for (const auto& s : blueprint.sections) {
                std::fprintf(stderr,
                    "    - %s (%d bars, energy=%d, density=%.1f, complexity=%.1f)\n",
                    s.name.c_str(), s.bars,
                    static_cast<int>(s.energy),
                    static_cast<double>(s.density),
                    static_cast<double>(s.complexity));
            }
            std::fprintf(stderr, "  instruments:\n");
            for (const auto& inst : blueprint.instruments) {
                std::fprintf(stderr, "    - %s (role: %s, GM: %d)\n",
                             inst.name.c_str(), inst.role.c_str(), inst.program);
            }

            std::fprintf(stderr, "\n=== Timeline ===\n");
            for (const auto& sec : timeline.sections) {
                std::fprintf(stderr,
                    "  %s: bars %d-%d (%d bars, energy=%d, density=%.1f)\n",
                    sec.name.c_str(), sec.start_bar, sec.end_bar,
                    sec.bars, static_cast<int>(sec.energy),
                    static_cast<double>(sec.density));
            }
            if (!timeline.transitions.empty()) {
                std::fprintf(stderr, "  transitions:\n");
                for (const auto& tr : timeline.transitions) {
                    const char* from_name = "?";
                    const char* to_name   = "?";
                    if (tr.from_section_index >= 0 &&
                        static_cast<std::size_t>(tr.from_section_index) <
                            timeline.sections.size())
                        from_name =
                            timeline.sections[tr.from_section_index].name.c_str();
                    if (tr.to_section_index >= 0 &&
                        static_cast<std::size_t>(tr.to_section_index) <
                            timeline.sections.size())
                        to_name =
                            timeline.sections[tr.to_section_index].name.c_str();
                    std::fprintf(stderr, "    %s -> %s (type: %s, bars: %d)\n",
                                 from_name, to_name,
                                 tr.type.c_str(), tr.transition_bars);
                }
            }
            std::fprintf(stderr, "  total_bars: %d, total_ticks: %d\n",
                         timeline.total_bars, timeline.total_ticks);

            std::fprintf(stderr, "\n=== Arrangement ===\n");
            for (const auto& si : arrangement.section_instruments) {
                std::fprintf(stderr, "  section '%s': ",
                             si.section_name.c_str());
                for (std::size_t ai = 0; ai < si.active_instruments.size();
                     ++ai) {
                    if (ai > 0) std::fprintf(stderr, ", ");
                    std::fprintf(stderr, "%s (%s)",
                                 si.active_instruments[ai].name.c_str(),
                                 si.active_instruments[ai].role.c_str());
                }
                std::fprintf(stderr, "\n");
            }

            if (instrument_mapper) {
                std::fprintf(stderr, "\n=== Instrument GM Mappings ===\n");
                for (const auto& inst : arrangement.all_instruments) {
                    auto gm = instrument_mapper->map_to_gm(inst.name, inst.role);
                    std::fprintf(stderr,
                        "  %s (%s) -> GM %d (%s / %s)\n",
                        inst.name.c_str(), inst.role.c_str(),
                        gm.program, gm.family.c_str(), gm.name.c_str());
                }
            }
        }
    } else {
        // ---- Legacy (backward compatible) ----------------------------
        resolved_key   = args.key;
        resolved_scale = args.scale;
        resolved_bpm   = args.bpm;
        resolved_bars  = args.bars;
    }

    // =======================================================================
    // MTE event generation (common to both paths)
    // =======================================================================

    int root_pc = key_to_root_pc(resolved_key);
    if (root_pc < 0) {
        std::fprintf(stderr, "Error: unknown key '%s'\n", resolved_key.c_str());
        return 1;
    }
    (void)root_pc;

    // ---- a. ScaleProvider ------------------------------------------------
    auto scales = make_scale_provider();
    if (!scales->knows(resolved_scale)) {
        std::fprintf(stderr, "Error: unknown scale '%s'\n",
                     resolved_scale.c_str());
        return 1;
    }

    // ---- b. ChordEngine --------------------------------------------------
    auto chord_engine = make_chord_engine();

    // ---- c. HarmonyEngine (DI via shared_ptr) ----------------------------
    std::shared_ptr<IScaleProvider> shared_scales = std::move(scales);
    std::shared_ptr<IChordEngine>   shared_chords = std::move(chord_engine);
    auto harmony = make_harmony_engine(shared_scales, shared_chords);

    // ---- d. Generate harmony ---------------------------------------------
    HarmonyRequest harm_req{resolved_key, resolved_scale, resolved_bars,
                            resolved_seed};
    auto harmony_events = harmony->generate(harm_req);

    // ---- e. Convert harmony events -> ChordSpecs -------------------------
    std::map<std::uint32_t, std::vector<MidiEvent>> chord_groups;
    for (const auto& ev : harmony_events) {
        chord_groups[ev.tick_on].push_back(ev);
    }

    constexpr int kTicksPerBar = kPpq * 4;

    std::vector<std::uint32_t> tick_ons;
    tick_ons.reserve(chord_groups.size());
    for (const auto& [tick, _] : chord_groups) {
        tick_ons.push_back(tick);
    }

    std::vector<ChordSpec> chord_specs;
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
        chord_specs.push_back({tick_ons[i], duration, detection.root_pc,
                               detection.quality, 0});
    }

    ChordRequest chord_req{chord_specs, resolved_seed, kPpq};

    // ---- f. BassEngine ---------------------------------------------------
    auto bass = make_bass_engine();
    BassStyle bass_style{
        .seed = static_cast<int>(resolved_seed), .use_octave_jump = false};
    auto bass_events = bass->generate(chord_req, bass_style);

    // ---- g. RhythmEngine -------------------------------------------------
    auto rhythm = make_rhythm_engine();
    RhythmStyle rhythm_style{
        .resolution = RhythmResolution::eighth,
        .swing      = 0.0f,
        .seed       = resolved_seed};
    FillSpec no_fill{-1, 1};
    auto rhythm_events = rhythm->generate(chord_req, rhythm_style, no_fill);

    // ---- h. DrumEngine ---------------------------------------------------
    auto drums = make_drum_engine();
    DrumStyle drum_style{
        .seed = static_cast<int>(resolved_seed), .use_ghosts = true,
        .use_open_hh = false};
    auto drum_events = drums->generate(chord_req, drum_style);

    // ---- i. PianoEngine --------------------------------------------------
    auto piano = make_piano_engine();
    PianoRequest piano_req{
        .chord_req     = chord_req,
        .style         = PianoStyle::block_chords,
        .pedal         = {},
        .bass_octave   = 3,
        .treble_octave = 4,
    };
    auto piano_events = piano->generate(piano_req);

    // ---- j. HumanizationEngine -------------------------------------------
    auto human = make_humanization_engine();
    HumanizationParams human_params{
        .seed                = resolved_seed,
        .timing_jitter_ticks = 3,
        .velocity_variation  = 5,
        .groove_swing        = 0.0f,
        .snap_to_grid        = false,
        .grid_resolution     = 120,
    };
    human->apply(piano_events, human_params);
    human->apply(bass_events, human_params);

    // ---- k. Combine drums (Rhythm + Drum engines) ------------------------
    std::vector<MidiEvent> drums_combined;
    drums_combined.reserve(rhythm_events.size() + drum_events.size());
    drums_combined.insert(drums_combined.end(), rhythm_events.begin(),
                          rhythm_events.end());
    drums_combined.insert(drums_combined.end(), drum_events.begin(),
                          drum_events.end());
    std::sort(drums_combined.begin(), drums_combined.end(),
              [](const MidiEvent& a, const MidiEvent& b) {
                  return a.tick_on < b.tick_on;
              });

    // ---- l. Build tracks ------------------------------------------------
    std::vector<MidiTrack> tracks;

    if (!args.prompt.empty()) {
        // Prompt path: create tracks from arrangement instruments,
        // mapping roles to generated events.
        bool piano_assigned = false;
        bool bass_assigned  = false;
        bool drums_assigned = false;

        for (const auto& inst : arrangement.all_instruments) {
            std::vector<MidiEvent> track_events;

            if (!piano_assigned &&
                (inst.role == "piano" || inst.role == "chords" ||
                 inst.role == "harmony" || inst.role == "rhythm")) {
                track_events = std::move(piano_events);
                piano_assigned = true;
            } else if (!bass_assigned && inst.role == "bass") {
                track_events = std::move(bass_events);
                bass_assigned = true;
            } else if (!drums_assigned &&
                       (inst.role == "drums" || inst.role == "percussion")) {
                track_events = std::move(drums_combined);
                drums_assigned = true;
            }

            if (!track_events.empty()) {
                tracks.push_back({inst.name, std::move(track_events)});
            }
        }

        // Fallback: append any unassigned core tracks
        if (!piano_assigned && !piano_events.empty())
            tracks.push_back({"Piano", std::move(piano_events)});
        if (!bass_assigned && !bass_events.empty())
            tracks.push_back({"Bass", std::move(bass_events)});
        if (!drums_assigned && !drums_combined.empty())
            tracks.push_back({"Drums", std::move(drums_combined)});
    } else {
        // Legacy tracks
        tracks = {
            {"Piano", std::move(piano_events)},
            {"Bass",  std::move(bass_events)},
            {"Drums", std::move(drums_combined)},
        };
    }

    // ---- m. Render to SMF ------------------------------------------------
    auto renderer = make_midi_renderer();
    SmfComposition comp;
    comp.ppq          = kPpq;
    comp.bpm          = resolved_bpm;
    comp.key          = resolved_key;
    comp.scale        = resolved_scale;
    comp.time_sig_num = 4;
    comp.time_sig_den = 4;
    comp.tracks       = std::move(tracks);

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
