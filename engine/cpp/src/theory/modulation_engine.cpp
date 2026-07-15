#include <aimidi/theory/IModulationEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>

#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

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
    return names[pc % 12];
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
    return std::string(q).c_str();
}

// ---------------------------------------------------------------------------
// Key signature helpers
// ---------------------------------------------------------------------------

// Number of accidentals for a major key.  Positive = sharps, negative = flats.
int major_key_accidentals(std::string_view key) {
    // Circle of fifths from C:
    // Sharps:  C(0) -> G(1) -> D(2) -> A(3) -> E(4) -> B(5) -> F#(6) -> C#(7)
    // Flats:   C(0) -> F(1) -> Bb(2) -> Eb(3) -> Ab(4) -> Db(5) -> Gb(6) -> Cb(7)
    static const std::unordered_map<std::string_view, int> sig = {
        {"Cb", -7}, {"Gb", -6}, {"Db", -5}, {"Ab", -4}, {"Eb", -3},
        {"Bb", -2}, {"F",  -1},
        {"C",   0},
        {"G",   1}, {"D",   2}, {"A",   3}, {"E",   4}, {"B",   5},
        {"F#",  6}, {"C#",  7},
    };
    auto it = sig.find(key);
    return it != sig.end() ? it->second : 0;
}

// All major keys with their accidentals.
const std::vector<std::pair<int, std::string>>& all_major_keys() {
    static const std::vector<std::pair<int, std::string>> keys = {
        {-7, "Cb"}, {-6, "Gb"}, {-5, "Db"}, {-4, "Ab"}, {-3, "Eb"},
        {-2, "Bb"}, {-1, "F"},
        {0, "C"},
        {1, "G"}, {2, "D"}, {3, "A"}, {4, "E"}, {5, "B"},
        {6, "F#"}, {7, "C#"},
    };
    return keys;
}

// Relative minor of each major key (natural minor).
std::string relative_minor_of(std::string_view major_key) {
    // A minor third above the major root.
    int pc = parse_root_pc(major_key);
    int minor_pc = (pc + 9) % 12; // down a minor third = up a major sixth

    auto acc = major_key_accidentals(major_key);
    // Choose sharp or flat spelling consistent with the key signature.
    if (acc >= 0) {
        // Sharp keys: use sharp spelling for minor
        return pc_name_sharp(minor_pc);
    } else {
        // Flat keys: minor may use flat; look up in relative table.
        static const std::unordered_map<std::string_view, std::string_view> rel = {
            {"Cb", "Ab"}, {"Gb", "Eb"}, {"Db", "Bb"}, {"Ab", "F"},
            {"Eb", "C"},  {"Bb", "G"},  {"F",  "D"},
            {"C",  "A"},  {"G",  "E"},  {"D",  "B"},
            {"A",  "F#"}, {"E",  "C#"}, {"B",  "G#"},
            {"F#", "D#"}, {"C#", "A#"},
        };
        auto it = rel.find(major_key);
        if (it != rel.end()) {
            return std::string(it->second);
        }
        return pc_name_sharp(minor_pc); // fallback
    }
}

int key_accidentals(std::string_view key, std::string_view scale) {
    if (scale == "minor" || scale == "natural_minor" ||
        scale == "harmonic_minor" || scale == "melodic_minor") {
        // For minor keys, find the relative major.
        // Relative major is up a minor third (3 semitones).
        int pc = parse_root_pc(key);
        int major_pc = (pc + 3) % 12;
        // Guess the major key name from the pitch class.
        // Use the same accidental type as the given key.
        const char* guess = pc_name_sharp(major_pc);
        // Try sharp table first.
        int acc = major_key_accidentals(guess);
        return acc;
    }
    return major_key_accidentals(key);
}

// ---------------------------------------------------------------------------
// Diatonic chord helpers
// ---------------------------------------------------------------------------

struct DiatonicChord {
    int         root_pc;
    const char* quality;
};

// Qualities for each degree of common scale types.
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

    if (!scales) {
        return result;
    }

    std::span<const int> intervals = scales->intervals_of(scale);
    if (intervals.empty()) {
        intervals = scales->intervals_of("major");
    }
    if (intervals.empty()) {
        return result;
    }

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

// Build a human-readable chord name from root_pc + quality.
std::string chord_name(int root_pc, std::string_view quality) {
    return std::string(pc_name_sharp(root_pc)) + quality_suffix(quality);
}

// Parse a chord name like "G7", "Am", "Cmaj7" into (root_pc, quality).
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
// ModulationEngine
// ---------------------------------------------------------------------------

class ModulationEngine final : public IModulationEngine {
public:
    explicit ModulationEngine(std::shared_ptr<IScaleProvider> scales)
        : scales_(std::move(scales)) {}

    std::string find_pivot_chord(
        std::string_view from_key, std::string_view from_scale,
        std::string_view to_key, std::string_view to_scale) const override {

        auto from_chords = diatonic_chords(from_key, from_scale, scales_.get());
        auto to_chords   = diatonic_chords(to_key,   to_scale,   scales_.get());

        if (from_chords.empty() || to_chords.empty()) {
            return {};
        }

        // Build a set of (root_pc, quality) pairs for the target key.
        struct PcQual { int pc; std::string_view q; };
        std::unordered_map<int, std::vector<std::string_view>> target_map;
        for (const auto& tc : to_chords) {
            target_map[tc.root_pc].push_back(tc.quality);
        }

        // Determine V of target key (strongest pivot candidate).
        int to_root = parse_root_pc(to_key);
        auto to_intervals = scales_->intervals_of(to_scale);
        if (to_intervals.empty()) {
            to_intervals = scales_->intervals_of("major");
        }
        int v_pc = -1;
        if (to_intervals.size() >= 5) {
            v_pc = (to_root + to_intervals[4]) % 12;
        }

        // Pass 1: prefer chords that share both root AND quality,
        //         and are the V of the target key (strongest pivot).
        for (const auto& fc : from_chords) {
            auto it = target_map.find(fc.root_pc);
            if (it != target_map.end()) {
                for (const auto& tq : it->second) {
                    if (fc.quality == tq && fc.root_pc == v_pc) {
                        return chord_name(fc.root_pc, fc.quality);
                    }
                }
            }
        }

        // Pass 2: any chord that shares both root AND quality.
        for (const auto& fc : from_chords) {
            auto it = target_map.find(fc.root_pc);
            if (it != target_map.end()) {
                for (const auto& tq : it->second) {
                    if (fc.quality == tq) {
                        return chord_name(fc.root_pc, fc.quality);
                    }
                }
            }
        }

        // Pass 3: any common root (even if qualities differ).
        std::unordered_set<int> from_roots;
        for (const auto& fc : from_chords) {
            from_roots.insert(fc.root_pc);
        }
        for (const auto& tc : to_chords) {
            if (from_roots.count(tc.root_pc)) {
                return chord_name(tc.root_pc, tc.quality);
            }
        }

        // Pass 4: enharmonic shortcut — try respelling.
        // Enharmonic notes share the same pitch class, so the common-root
        // detection above already handles them.  The distinction matters only
        // for the chord name returned (e.g. respell C# as Db).
        // If we reach here, no common diatonic pivot exists.
        return {};
    }

    std::vector<std::pair<int, std::string>> generate_modulation(
        std::string_view from_key, std::string_view from_scale,
        std::string_view to_key, std::string_view to_scale,
        int transition_bars, Seed seed) const override {

        std::mt19937_64 rng(seed);
        (void)rng;
        std::vector<std::pair<int, std::string>> result;

        if (transition_bars <= 0) {
            return result;
        }

        int to_root_pc = parse_root_pc(to_key);

        // Find the pivot chord.
        std::string pivot = find_pivot_chord(from_key, from_scale,
                                              to_key, to_scale);
        bool has_pivot = !pivot.empty();

        // Determine degrees in target key.
        auto to_intervals = scales_ ?
            scales_->intervals_for(to_scale, to_root_pc) :
            std::vector<int>{};

        if (to_intervals.size() < 3) {
            // Fallback: just emit the pivot if available.
            if (has_pivot) {
                auto [pc, qual] = parse_chord_name(pivot);
                result.emplace_back(pc, qual);
            }
            return result;
        }

        const char** qualities = qualities_for_scale(to_scale);

        int bars_left = transition_bars;

        // Bar 1: pivot chord (if found).
        if (bars_left > 0 && has_pivot) {
            auto [pc, qual] = parse_chord_name(pivot);
            result.emplace_back(pc, qual);
            --bars_left;
        }

        // Bars 2+: II-V-I cadence in the target key.
        if (bars_left > 0 && to_intervals.size() >= 5) {
            int ii_pc = to_intervals[1] % 12;
            int v_pc  = to_intervals[4] % 12;
            int i_pc  = to_intervals[0] % 12;

            // Determine qualities.
            bool is_major = (to_scale == "major");
            bool is_harm = (to_scale == "harmonic_minor");

            // In harmonic minor, V is major (dominant).
            const char* ii_q = qualities[1 % 7];
            const char* v_q  = qualities[4 % 7];
            const char* i_q  = qualities[0 % 7];

            // Use dominant 7th for V in major/harmonic_minor when more bars available.
            if (is_major || is_harm) {
                if (bars_left > 1) {
                    v_q = "7";
                }
            }

            // Emit II.
            result.emplace_back(ii_pc, ii_q);
            --bars_left;

            // Emit V (or V7 if enough bars).
            if (bars_left > 0) {
                result.emplace_back(v_pc, v_q);
                --bars_left;
            }

            // Emit I (tonic resolution) if bars remain.
            if (bars_left > 0) {
                result.emplace_back(i_pc, i_q);
                --bars_left;
            }
        }

        return result;
    }

    bool are_closely_related(
        std::string_view key1, std::string_view scale1,
        std::string_view key2, std::string_view scale2) const override {

        int acc1 = key_accidentals(key1, scale1);
        int acc2 = key_accidentals(key2, scale2);
        return std::abs(acc1 - acc2) <= 1;
    }

    std::vector<std::string> closely_related_keys(
        std::string_view key, std::string_view scale) const override {

        int base_acc = key_accidentals(key, scale);
        std::vector<std::string> result;

        for (const auto& [acc, name] : all_major_keys()) {
            if (std::abs(acc - base_acc) <= 1) {
                result.push_back(name);
                // Also add the relative minor.
                result.push_back(relative_minor_of(name));
            }
        }

        return result;
    }

private:
    std::shared_ptr<IScaleProvider> scales_;
};

} // namespace

std::unique_ptr<IModulationEngine> make_modulation_engine(
    std::shared_ptr<IScaleProvider> scales) {
    if (!scales) {
        scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    }
    return std::make_unique<ModulationEngine>(std::move(scales));
}

} // namespace aimidi::theory
