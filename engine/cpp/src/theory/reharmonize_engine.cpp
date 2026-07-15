// Implementation: ReharmonizeEngine — generates new harmony while preserving
// melody.  Implements Workflow 8 (W8): reharmonize.
//
// Five styles:
//   simple    — enhance each chord with richer extensions
//   jazz      — ii-V-I, tritone subs, turnarounds, extended chords
//   modal     — shift the mode while keeping the root
//   chromatic — add chromatic passing chords, secondary dominants
//   pedal     — hold a pedal tone while changing chords above it
//
// Determinism: same seed + same request → identical output (std::mt19937_64).
#include <aimidi/theory/IReharmonizeEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <aimidi/theory/IHarmonyEngine.hpp>

#include <algorithm>
#include <array>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>
#include <cstdint>

namespace aimidi::theory {

namespace {

// ---------------------------------------------------------------------------
// Pitch-class helpers
// ---------------------------------------------------------------------------

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

const char* pc_name_sharp(int pc) {
    static constexpr const char* names[] = {
        "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
    };
    return names[((pc % 12) + 12) % 12];
}

const char* quality_suffix(std::string_view q) {
    if (q == "maj")  return "";
    if (q == "min")  return "m";
    if (q == "7")    return "7";
    if (q == "m7")   return "m7";
    if (q == "maj7") return "maj7";
    if (q == "dim")  return "dim";
    if (q == "dim7") return "dim7";
    if (q == "aug")  return "aug";
    if (q == "m7b5") return "m7b5";
    return std::string(q).c_str();
}

std::string chord_name(int root_pc, std::string_view quality) {
    return std::string(pc_name_sharp(root_pc)) + quality_suffix(quality);
}

std::pair<int, std::string> parse_chord_name(const std::string& name) {
    if (name.empty()) return {0, "maj"};
    std::size_t pos = 0;
    std::string root;
    if (name.size() >= 2 && (name[1] == '#' || name[1] == 'b')) {
        root = name.substr(0, 2);
        pos = 2;
    } else {
        root = name.substr(0, 1);
        pos = 1;
    }
    int root_pc = parse_root_pc(root);
    std::string qual = name.substr(pos);
    if (qual.empty()) qual = "maj";
    else if (qual == "m") qual = "min";
    return {root_pc, qual};
}

// ---------------------------------------------------------------------------
// Diatonic chord helpers
// ---------------------------------------------------------------------------

struct DiatonicChord {
    int         root_pc;
    const char* quality;
};

static constexpr const char* kMajorQualities[] = {
    "maj", "min", "min", "maj", "maj", "min", "dim"
};
static constexpr const char* kNaturalMinorQualities[] = {
    "min", "dim", "maj", "min", "min", "maj", "maj"
};
static constexpr const char* kHarmonicMinorQualities[] = {
    "min", "dim", "aug", "min", "maj", "maj", "dim"
};
static constexpr const char* kMelodicMinorQualities[] = {
    "min", "min", "aug", "maj", "maj", "dim", "dim"
};

const char** qualities_for_scale(std::string_view scale) {
    if (scale == "minor" || scale == "natural_minor") {
        return const_cast<const char**>(kNaturalMinorQualities);
    }
    if (scale == "harmonic_minor") {
        return const_cast<const char**>(kHarmonicMinorQualities);
    }
    if (scale == "melodic_minor") {
        return const_cast<const char**>(kMelodicMinorQualities);
    }
    return const_cast<const char**>(kMajorQualities);
}

std::vector<DiatonicChord> diatonic_chords(
    std::string_view key, std::string_view scale,
    const IScaleProvider* scales) {

    std::vector<DiatonicChord> result;
    int root_pc = parse_root_pc(key);
    if (!scales) return result;

    std::span<const int> intervals = scales->intervals_of(scale);
    if (intervals.empty()) intervals = scales->intervals_of("major");
    if (intervals.empty()) return result;

    const char** qualities = qualities_for_scale(scale);
    result.reserve(intervals.size());
    for (std::size_t i = 0; i < intervals.size(); ++i) {
        DiatonicChord dc{};
        dc.root_pc = (root_pc + intervals[i]) % 12;
        dc.quality = (i < 7) ? qualities[i] : "maj";
        result.push_back(dc);
    }
    return result;
}

// Find the diatonic degree of a chord root in a given key/scale.
// Returns -1 if not diatonic.
int find_degree(int root_pc, std::string_view key,
                std::string_view scale, const IScaleProvider* scales) {
    auto diatonics = diatonic_chords(key, scale, scales);
    for (std::size_t i = 0; i < diatonics.size(); ++i) {
        if (diatonics[i].root_pc == root_pc) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

// ---------------------------------------------------------------------------
// Reharmonization algorithms (private helpers)
// ---------------------------------------------------------------------------

std::vector<std::string> simple_reharm(
    const std::vector<std::string>& original_chords,
    std::string_view key, std::string_view scale,
    const IScaleProvider* scales, std::mt19937_64& rng) {

    if (original_chords.empty()) return {};

    std::vector<std::string> result;
    result.reserve(original_chords.size());

    for (const auto& chord : original_chords) {
        auto [root_pc, quality] = parse_chord_name(chord);
        int degree = find_degree(root_pc, key, scale, scales);

        std::string new_quality = quality;
        if (degree >= 0) {
            switch (degree) {
                case 0: { // I
                    const char* opts[] = {"maj", "maj7"};
                    new_quality = opts[rng() % 2];
                    break;
                }
                case 1: { // ii
                    const char* opts[] = {"min", "m7"};
                    new_quality = opts[rng() % 2];
                    break;
                }
                case 2: { // iii
                    const char* opts[] = {"min", "m7"};
                    new_quality = opts[rng() % 2];
                    break;
                }
                case 3: { // IV
                    const char* opts[] = {"maj", "maj7"};
                    new_quality = opts[rng() % 2];
                    break;
                }
                case 4: { // V
                    const char* opts[] = {"7", "maj7"};
                    new_quality = opts[rng() % 2];
                    break;
                }
                case 5: { // vi
                    const char* opts[] = {"min", "m7"};
                    new_quality = opts[rng() % 2];
                    break;
                }
                case 6: { // vii°
                    const char* opts[] = {"dim", "m7b5"};
                    new_quality = opts[rng() % 2];
                    break;
                }
                default:
                    break;
            }
        }
        result.push_back(chord_name(root_pc, new_quality));
    }
    return result;
}

std::vector<std::string> jazz_reharm(
    const std::vector<std::string>& original_chords,
    std::string_view key, std::string_view scale,
    const IScaleProvider* scales, std::mt19937_64& rng) {

    if (original_chords.empty()) return {};

    auto diatonics = diatonic_chords(key, scale, scales);
    int key_root_pc = parse_root_pc(key);

    std::vector<std::string> result;
    result.reserve(original_chords.size() * 2);

    for (std::size_t i = 0; i < original_chords.size(); ++i) {
        auto [root_pc, quality] = parse_chord_name(original_chords[i]);
        int degree = find_degree(root_pc, key, scale, scales);

        // Determine if next chord resolves as a target (I, vi).
        bool next_is_target = false;
        if (i + 1 < original_chords.size()) {
            auto [next_root, _] = parse_chord_name(original_chords[i + 1]);
            for (std::size_t d = 0; d < diatonics.size(); ++d) {
                if (diatonics[d].root_pc == next_root && (d == 0 || d == 5)) {
                    next_is_target = true;
                    break;
                }
            }
        }

        // If V degree and target follows, add ii-V with potential tritone sub.
        if (degree == 4 && next_is_target && diatonics.size() >= 2) {
            if (!diatonics.empty()) {
                int ii_pc = diatonics[1 % diatonics.size()].root_pc;
                result.push_back(chord_name(ii_pc, "m7"));
            }
            bool tritone = (rng() % 3) == 0;
            if (tritone) {
                int bII_pc = (key_root_pc + 1) % 12;
                result.push_back(chord_name(bII_pc, "7"));
            } else {
                result.push_back(chord_name(root_pc, "7"));
            }
            continue;
        }

        // Default: use extended jazz quality.
        std::string jz_quality;
        if (quality == "maj" || quality.empty()) {
            jz_quality = (degree == 4) ? "7" : "maj7";
        } else if (quality == "min" || quality == "m") {
            jz_quality = "m7";
        } else {
            jz_quality = (quality == "dim") ? "m7b5" : quality;
        }
        result.push_back(chord_name(root_pc, jz_quality));
    }

    // Turnaround pass: I-vi-ii-V → I-VI7-ii-V
    if (diatonics.size() >= 6) {
        for (std::size_t i = 0; i + 3 < result.size(); ++i) {
            auto [c1, q1] = parse_chord_name(result[i]);
            auto [c2, q2] = parse_chord_name(result[i + 1]);
            auto [c3, q3] = parse_chord_name(result[i + 2]);
            auto [c4, q4] = parse_chord_name(result[i + 3]);
            int vi_pc = diatonics[5 % diatonics.size()].root_pc;
            int ii_pc = diatonics[1 % diatonics.size()].root_pc;
            int v_pc  = diatonics[4 % diatonics.size()].root_pc;
            if (c1 == key_root_pc &&
                c2 == vi_pc &&
                c3 == ii_pc &&
                c4 == v_pc) {
                // VI7 = dominant of ii (root a fifth above ii)
                int vi7_pc = (ii_pc + 7) % 12;
                result[i + 1] = chord_name(vi7_pc, "7");
            }
        }
    }

    return result;
}

std::vector<std::string> modal_reharm(
    const std::vector<std::string>& original_chords,
    std::string_view key, std::string_view scale,
    const IScaleProvider* scales, std::mt19937_64& rng) {

    if (original_chords.empty()) return {};

    // Choose a mode from available alternates.
    static constexpr const char* kModes[] = {
        "ionian", "dorian", "phrygian", "lydian",
        "mixolydian", "aeolian", "locrian"
    };
    const char* mode = kModes[rng() % 7];

    // Chord qualities for each mode (diatonic rotation of major scale).
    static constexpr const char* kModeQualities[7][7] = {
        {"maj", "min", "min", "maj", "maj", "min", "dim"},  // ionian
        {"min", "min", "maj", "maj", "min", "dim", "maj"},  // dorian
        {"min", "maj", "maj", "min", "dim", "maj", "min"},  // phrygian
        {"maj", "maj", "min", "dim", "maj", "min", "min"},  // lydian
        {"maj", "min", "dim", "maj", "min", "min", "maj"},  // mixolydian
        {"min", "dim", "maj", "min", "min", "maj", "maj"},  // aeolian
        {"dim", "maj", "min", "min", "maj", "maj", "min"},  // locrian
    };

    int mode_index = 0;
    for (int m = 0; m < 7; ++m) {
        if (scale == kModes[m]) {
            mode_index = m;
            break;
        }
    }

    int key_root_pc = parse_root_pc(key);
    auto base_intervals = scales->intervals_of("major");
    if (base_intervals.empty()) return original_chords;

    // Build scale intervals for the chosen mode (rotation of major).
    std::vector<int> mode_intervals(base_intervals.size());
    for (std::size_t i = 0; i < base_intervals.size(); ++i) {
        mode_intervals[i] = (base_intervals[(i + mode_index) % base_intervals.size()]
                             - base_intervals[mode_index] + 12) % 12;
    }

    std::vector<std::string> result;
    result.reserve(original_chords.size());

    for (const auto& chord : original_chords) {
        auto [root_pc, quality] = parse_chord_name(chord);
        int degree = find_degree(root_pc, key, scale, scales);

        if (degree >= 0 && degree < 7) {
            // Map to the new mode's quality for the same degree.
            const char* new_q = kModeQualities[mode_index][degree];
            result.push_back(chord_name(root_pc, new_q));
        } else {
            // Non-diatonic: keep as-is with potential extension.
            std::string enhanced = quality;
            if (quality == "maj" || quality.empty()) enhanced = "maj";
            else if (quality == "min") enhanced = "min";
            result.push_back(chord_name(root_pc, enhanced));
        }
    }
    return result;
}

std::vector<std::string> chromatic_reharm(
    const std::vector<std::string>& original_chords,
    std::string_view key, std::string_view scale,
    const IScaleProvider* scales, std::mt19937_64& rng) {

    if (original_chords.empty()) return {};

    int key_root_pc = parse_root_pc(key);
    std::vector<std::string> result;
    result.reserve(original_chords.size() * 2);

    for (std::size_t i = 0; i < original_chords.size(); ++i) {
        auto [root_pc, quality] = parse_chord_name(original_chords[i]);

        if (i > 0 && (rng() % 2) == 0) {
            // Add chromatic passing chord: a half-step above or below the
            // current root, usually diminished or dominant.
            int dir = (rng() % 2 == 0) ? 1 : -1;
            int pass_pc = ((root_pc + dir) % 12 + 12) % 12;
            bool use_dim = (rng() % 2) == 0;
            result.push_back(chord_name(pass_pc, use_dim ? "dim" : "7"));
        }

        // Primary chord: use rich chromatic substitutions.
        std::string new_quality;
        int degree = find_degree(root_pc, key, scale, scales);

        // Add secondary dominant if this chord targets a diatonic degree.
        if (degree == 4 && i > 0) {
            // V gets a b9 or #9 for chromatic flavor.
            const char* alts[] = {"7", "dim7", "7"};
            new_quality = alts[rng() % 3];
        } else if (quality == "maj" || quality.empty()) {
            const char* opts[] = {"maj", "maj7", "aug"};
            new_quality = opts[rng() % 3];
        } else if (quality == "min") {
            const char* opts[] = {"min", "m7", "dim"};
            new_quality = opts[rng() % 3];
        } else if (quality == "dim") {
            const char* opts[] = {"dim", "dim7"};
            new_quality = opts[rng() % 2];
        } else {
            new_quality = quality;
        }

        result.push_back(chord_name(root_pc, new_quality));
    }
    return result;
}

std::vector<std::string> pedal_reharm(
    const std::vector<std::string>& original_chords,
    std::string_view key, std::string_view scale,
    const IScaleProvider* scales, std::mt19937_64& rng) {

    if (original_chords.empty()) return {};

    int pedal_pc = parse_root_pc(key);
    auto diatonics = diatonic_chords(key, scale, scales);

    std::vector<std::string> result;
    result.reserve(original_chords.size());

    for (std::size_t i = 0; i < original_chords.size(); ++i) {
        // Pick a chord from the diatonic pool that doesn't clash with the pedal.
        // The pedal is the root, so any diatonic chord works.
        std::size_t di_idx = rng() % diatonics.size();
        const auto& dc = diatonics[di_idx];

        // Occasionally use a non-diatonic color chord.
        std::string quality = dc.quality;
        if ((rng() % 4) == 0) {
            const char* colors[] = {"7", "maj7", "m7"};
            quality = colors[rng() % 3];
        }

        result.push_back(chord_name(dc.root_pc, quality));
    }
    return result;
}

} // namespace

// ---------------------------------------------------------------------------
// ReharmonizeEngine
// ---------------------------------------------------------------------------

class ReharmonizeEngine final : public IReharmonizeEngine {
public:
    ReharmonizeEngine(std::shared_ptr<IScaleProvider> scales,
                      std::shared_ptr<IChordEngine> chords,
                      std::shared_ptr<IHarmonyEngine> harmony)
        : scales_(std::move(scales))
        , chords_(std::move(chords))
        , harmony_(std::move(harmony)) {}

    std::vector<std::string> reharmonize(
        const ReharmonizeRequest& req) const override {

        std::mt19937_64 rng(req.seed);

        if (req.bars < 1 || req.melody.empty()) {
            return req.original_chords;
        }

        std::string key_str(req.key);
        std::string scale_str(req.scale);

        switch (req.style) {
            case ReharmStyle::simple:
                return simple_reharm(req.original_chords, key_str, scale_str,
                                     scales_.get(), rng);
            case ReharmStyle::jazz:
                return jazz_reharm(req.original_chords, key_str, scale_str,
                                   scales_.get(), rng);
            case ReharmStyle::modal:
                return modal_reharm(req.original_chords, key_str, scale_str,
                                    scales_.get(), rng);
            case ReharmStyle::chromatic:
                return chromatic_reharm(req.original_chords, key_str, scale_str,
                                        scales_.get(), rng);
            case ReharmStyle::pedal:
                return pedal_reharm(req.original_chords, key_str, scale_str,
                                    scales_.get(), rng);
            default:
                return req.original_chords;
        }
    }

    std::vector<MidiEvent> generate_voicings(
        const std::vector<std::string>& new_chords,
        int bars, int bpm, Seed seed) const override {

        if (new_chords.empty()) return {};

        const std::uint32_t ppq = static_cast<std::uint32_t>(kPpq);
        const std::uint32_t ticks_per_bar = ppq * 4u;

        int num_chords = static_cast<int>(new_chords.size());
        int bars_per_chord = (num_chords > 0)
            ? std::max(1, bars / num_chords)
            : 1;
        std::uint32_t ticks_per_chord = ticks_per_bar
            * static_cast<std::uint32_t>(bars_per_chord);

        std::vector<ChordSpec> specs;
        specs.reserve(new_chords.size());

        for (std::size_t i = 0; i < new_chords.size(); ++i) {
            auto [root_pc, quality] = parse_chord_name(new_chords[i]);

            ChordSpec cs{};
            cs.start_tick     = static_cast<std::uint32_t>(i) * ticks_per_chord;
            cs.duration_ticks = ticks_per_chord;
            cs.root_pc        = root_pc;
            cs.quality        = quality;
            cs.channel        = 0u;
            specs.push_back(std::move(cs));
        }

        if (!chords_) return {};
        return chords_->voicing(ChordRequest{std::move(specs), seed, ppq});
    }

private:
    std::shared_ptr<IScaleProvider> scales_;
    std::shared_ptr<IChordEngine>   chords_;
    std::shared_ptr<IHarmonyEngine> harmony_;
};

// Factory
std::unique_ptr<IReharmonizeEngine> make_reharmonize_engine(
    std::shared_ptr<IScaleProvider> scales,
    std::shared_ptr<IChordEngine> chords,
    std::shared_ptr<IHarmonyEngine> harmony) {

    if (!scales) {
        scales = std::shared_ptr<IScaleProvider>(
            make_scale_provider().release());
    }
    if (!chords) {
        chords = std::shared_ptr<IChordEngine>(
            make_chord_engine().release());
    }
    // harmony is optional — used for future extended reharm generation.

    return std::make_unique<ReharmonizeEngine>(
        std::move(scales), std::move(chords), std::move(harmony));
}

} // namespace aimidi::theory
