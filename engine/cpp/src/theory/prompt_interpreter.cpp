#include <aimidi/theory/IPromptInterpreter.hpp>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace aimidi::theory {

namespace {

// ---------------------------------------------------------------------------
//  String helpers
// ---------------------------------------------------------------------------

[[nodiscard]] std::string to_lower(std::string_view s) {
    std::string r;
    r.reserve(s.size());
    for (char c : s)
        r.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    return r;
}

[[nodiscard]] bool contains(std::string_view haystack, std::string_view needle) {
    return haystack.find(needle) != std::string_view::npos;
}

// FNV-1a 64-bit hash.
[[nodiscard]] std::uint64_t fnv1a(std::string_view s) {
    std::uint64_t h = 14695981039346656037ULL;
    for (unsigned char c : s) {
        h ^= c;
        h *= 1099511628211ULL;
    }
    return h;
}

// ---------------------------------------------------------------------------
//  Numeric extraction helpers
// ---------------------------------------------------------------------------

// Parse the first positive integer that appears right before or after "bpm".
// Returns 120 on failure.
[[nodiscard]] int detect_bpm(std::string_view lower) {
    std::size_t i = 0;
    while (i < lower.size()) {
        auto p = lower.find("bpm", i);
        if (p == std::string_view::npos)
            break;

        // --- digits before "bpm" (handles "140bpm" and "140 bpm") ---
        std::size_t start = p;
        while (start > 0 && lower[start - 1] == ' ')
            --start;
        while (start > 0 && lower[start - 1] >= '0' && lower[start - 1] <= '9')
            --start;

        if (start < p) {
            std::size_t end = p;
            while (end > start && lower[end - 1] == ' ')
                --end;
            if (start < end) {
                int val{};
                auto [ptr, ec] =
                    std::from_chars(lower.data() + start, lower.data() + end, val);
                (void)ptr;
                if (ec == std::errc{} && val >= 20 && val <= 300)
                    return val;
            }
        }

        // --- digits after "bpm" (handles "bpm 140") ---
        std::size_t after = p + 3;
        while (after < lower.size() && lower[after] == ' ')
            ++after;
        std::size_t n_end = after;
        while (n_end < lower.size() && lower[n_end] >= '0' && lower[n_end] <= '9')
            ++n_end;
        if (after < n_end) {
            int val{};
            auto [ptr, ec] =
                std::from_chars(lower.data() + after, lower.data() + n_end, val);
            (void)ptr;
            if (ec == std::errc{} && val >= 20 && val <= 300)
                return val;
        }

        i = p + 3;
    }
    return 120;
}

// Extract total bar count from "N bar(s)" pattern.  Returns -1 when absent.
[[nodiscard]] int extract_total_bars(std::string_view lower) {
    std::size_t i = 0;
    while (i < lower.size()) {
        auto p = lower.find(" bar", i);
        if (p == std::string_view::npos)
            break;

        std::size_t start = p;
        while (start > 0 && lower[start - 1] == ' ')
            --start;
        while (start > 0 && lower[start - 1] >= '0' && lower[start - 1] <= '9')
            --start;

        if (start < p) {
            std::size_t end = p;
            while (end > start && lower[end - 1] == ' ')
                --end;
            if (start < end) {
                int val{};
                auto [ptr, ec] =
                    std::from_chars(lower.data() + start, lower.data() + end, val);
                (void)ptr;
                if (ec == std::errc{} && val >= 1 && val <= 256)
                    return val;
            }
        }
        i = p + 4;
    }
    return -1;
}

// Extract per-section bar count:  try number after section name first, then
// "N bar" pattern before section name.
[[nodiscard]] int extract_bars_for_section(std::string_view lower,
                                           std::string_view section_name,
                                           int default_bars) {
    auto nlow = to_lower(section_name);
    auto pos  = lower.find(nlow);
    if (pos == std::string_view::npos)
        return default_bars;

    // "section N" or "section N bars"
    std::size_t after  = pos + nlow.size();
    while (after < lower.size() && lower[after] == ' ')
        ++after;
    std::size_t n_end = after;
    while (n_end < lower.size() && lower[n_end] >= '0' && lower[n_end] <= '9')
        ++n_end;
    if (after < n_end) {
        int val{};
        auto [ptr, ec] =
            std::from_chars(lower.data() + after, lower.data() + n_end, val);
        (void)ptr;
        if (ec == std::errc{} && val >= 1 && val <= 256)
            return val;
    }

    // "N bar section"
    std::size_t search_start = (pos > 20) ? pos - 20 : 0;
    auto window              = lower.substr(search_start, pos - search_start);
    auto bar_p               = window.rfind(" bar");
    if (bar_p != std::string_view::npos) {
        std::size_t num_end   = bar_p;
        std::size_t num_start = num_end;
        while (num_start > 0 && window[num_start - 1] == ' ')
            --num_start;
        while (num_start > 0 && window[num_start - 1] >= '0' &&
               window[num_start - 1] <= '9')
            --num_start;
        if (num_start < num_end) {
            std::size_t ae = num_end;
            while (ae > num_start && window[ae - 1] == ' ')
                --ae;
            if (num_start < ae) {
                int val{};
                auto [ptr, ec] = std::from_chars(window.data() + num_start,
                                                 window.data() + ae, val);
                (void)ptr;
                if (ec == std::errc{} && val >= 1 && val <= 256)
                    return val;
            }
        }
    }

    return default_bars;
}

// ---------------------------------------------------------------------------
//  Detection helpers
// ---------------------------------------------------------------------------

[[nodiscard]] bool has_literal_notes(std::string_view lower) {
    constexpr std::string_view patterns[] = {
        "note c4", "note d4", "note e4", "note f4", "note g4",
        "notes c", "notes d", "notes e", "notes f", "notes g",
        "notes a", "notes b", "midi notes", "write notes",
    };
    for (auto p : patterns)
        if (contains(lower, p))
            return true;
    return false;
}

[[nodiscard]] std::vector<std::string> detect_genre(std::string_view lower) {
    using Entry = std::pair<std::string_view, std::string_view>;
    constexpr Entry map[] = {
        {"pop",        "pop"},
        {"rock",       "rock"},
        {"jazz",       "jazz"},
        {"blues",      "blues"},
        {"electronic", "electronic"},
        {"edm",        "electronic"},
        {"techno",     "electronic"},
        {"hip hop",    "hip_hop"},
        {"rap",        "hip_hop"},
        {"rnb",        "rnb"},
        {"r&b",        "rnb"},
        {"funk",       "funk"},
        {"soul",       "soul"},
        {"classical",  "classical"},
        {"latin",      "latin"},
        {"salsa",      "latin"},
        {"bossa",      "latin"},
        {"country",    "country"},
        {"metal",      "metal"},
        {"folk",       "folk"},
        {"pop",        "pop"},
    };
    for (auto& [kw, genre] : map)
        if (contains(lower, kw))
            return {std::string(genre)};
    return {};
}

[[nodiscard]] std::string ucfirst(std::string_view s) {
    if (s.empty()) return {};
    std::string r(s);
    r[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(r[0])));
    return r;
}

[[nodiscard]] std::pair<std::string, std::string>
detect_key_and_scale(std::string_view lower) {
    // Natural notes
    constexpr std::string_view notes[]{"c", "d", "e", "f", "g", "a", "b"};
    // Accidentals
    constexpr std::string_view acc[]{"f#", "c#", "g#", "d#", "a#", "e#",
                                     "bb", "eb", "ab", "db", "gb"};
    constexpr std::string_view acc_disp[]{"F#", "C#", "G#", "D#", "A#", "E#",
                                          "Bb", "Eb", "Ab", "Db", "Gb"};

    // "key of X" / "key of Xm" (natural)
    for (auto n : notes) {
        auto k = std::string("key of ") + std::string(n);
        if (contains(lower, k))
            return {ucfirst(n), "major"};
        if (contains(lower, k + "m"))
            return {ucfirst(n), "minor"};
    }

    // "X minor" / "X major" (natural)
    for (auto n : notes) {
        if (contains(lower, std::string(n) + " minor"))
            return {ucfirst(n), "minor"};
        if (contains(lower, std::string(n) + " major"))
            return {ucfirst(n), "major"};
    }

    // "X minor" / "X major" (accidental)
    for (std::size_t i = 0; i < 11; ++i)
        if (contains(lower, std::string(acc[i]) + " minor"))
            return {std::string(acc_disp[i]), "minor"};
        else if (contains(lower, std::string(acc[i]) + " major"))
            return {std::string(acc_disp[i]), "major"};

    // "in Xm" / "in X" (natural)
    for (auto n : notes) {
        if (contains(lower, std::string("in ") + std::string(n) + "m"))
            return {ucfirst(n), "minor"};
    }
    for (auto n : notes) {
        if (contains(lower, std::string("in ") + std::string(n)))
            return {ucfirst(n), "major"};
    }

    // "in Xm" / "in X" / "key of X" (accidental)
    for (std::size_t i = 0; i < 11; ++i) {
        auto a = std::string(acc[i]);
        auto d = std::string(acc_disp[i]);
        if (contains(lower, std::string("key of ") + a))
            return {d, "major"};
        if (contains(lower, std::string("in ") + a + "m"))
            return {d, "minor"};
        if (contains(lower, std::string("in ") + a))
            return {d, "major"};
    }

    return {"C", "major"};
}

[[nodiscard]] EnergyLevel detect_energy(std::string_view lower) {
    constexpr std::string_view high_kw[] = {"upbeat", "energetic", "lively", "fast"};
    constexpr std::string_view low_kw[]  = {"calm",   "relaxed", "relaxing",
                                            "chill",  "slow",    "mellow"};
    constexpr std::string_view mid_kw[]  = {"moderate", "medium", "neutral"};

    for (auto kw : high_kw)
        if (contains(lower, kw))
            return EnergyLevel::high;
    for (auto kw : low_kw)
        if (contains(lower, kw))
            return EnergyLevel::low;
    for (auto kw : mid_kw)
        if (contains(lower, kw))
            return EnergyLevel::mid;
    return EnergyLevel::mid;
}

[[nodiscard]] Emotion emotion_from_energy(EnergyLevel e) {
    switch (e) {
    case EnergyLevel::high: return {0.8f, 0.7f, 0.6f};
    case EnergyLevel::mid:  return {0.5f, 0.5f, 0.5f};
    case EnergyLevel::low:  return {0.3f, 0.2f, 0.4f};
    default:                return {0.5f, 0.5f, 0.5f};
    }
}

[[nodiscard]] std::vector<BlueprintSection>
build_sections(std::string_view lower, EnergyLevel default_energy) {
    // --- Explicit section keywords --------------------------------------------
    bool has_intro  = contains(lower, "intro");
    bool has_verse  = contains(lower, "verse");
    bool has_chorus = contains(lower, "chorus");
    bool has_bridge = contains(lower, "bridge");
    bool has_outro  = contains(lower, "outro");

    if (has_intro || has_verse || has_chorus || has_bridge || has_outro) {
        std::vector<BlueprintSection> sections;
        auto add = [&](std::string_view name, int default_bars,
                       EnergyLevel energy) {
            int bars        = extract_bars_for_section(lower, name, default_bars);
            float density   = 0.5f;
            float complexity = 0.3f;
            if (energy == EnergyLevel::high) {
                density   = 0.8f;
                complexity = 0.6f;
            } else if (energy == EnergyLevel::low) {
                density   = 0.3f;
                complexity = 0.2f;
            }
            sections.push_back({std::string(name), bars, energy, density, complexity});
        };
        if (has_intro)
            add("intro", 2, EnergyLevel::low);
        if (has_verse)
            add("verse", 4, EnergyLevel::mid);
        if (has_chorus)
            add("chorus", 4, EnergyLevel::high);
        if (has_bridge)
            add("bridge", 4, EnergyLevel::mid);
        if (has_outro)
            add("outro", 2, EnergyLevel::low);
        return sections;
    }

    // --- No section keywords — use bar-count thresholds -----------------------
    int bars = extract_total_bars(lower);
    if (bars < 0) { // no bar count specified at all
        return {
            {"intro",  2, EnergyLevel::low,  0.3f, 0.2f},
            {"verse",  4, EnergyLevel::mid,  0.5f, 0.3f},
            {"chorus", 4, EnergyLevel::high, 0.8f, 0.5f},
            {"outro",  2, EnergyLevel::low,  0.3f, 0.2f},
        };
    }
    if (bars <= 4)
        return {{"verse", bars, default_energy, 0.5f, 0.3f}};
    if (bars >= 12)
        return {
            {"intro",  2, EnergyLevel::low,  0.3f, 0.2f},
            {"verse",  4, EnergyLevel::mid,  0.5f, 0.3f},
            {"chorus", 4, EnergyLevel::high, 0.8f, 0.5f},
            {"outro",  2, EnergyLevel::low,  0.3f, 0.2f},
        };
    if (bars >= 8)
        return {
            {"verse",  4, EnergyLevel::mid,  0.5f, 0.3f},
            {"chorus", 4, EnergyLevel::high, 0.8f, 0.5f},
        };
    return {{"verse", bars, default_energy, 0.5f, 0.3f}};
}

[[nodiscard]] std::vector<BlueprintInstrument>
detect_instruments(std::string_view lower) {
    bool has_piano   = contains(lower, "piano") || contains(lower, "keys");
    bool has_bass    = contains(lower, "bass");
    bool has_drums   = contains(lower, "drums") || contains(lower, "drum");
    bool has_guitar  = contains(lower, "guitar");
    bool has_strings = contains(lower, "strings");
    bool has_synth   = contains(lower, "synth");
    bool has_brass   = contains(lower, "brass");
    bool has_sax     = contains(lower, "sax");

    if (has_piano || has_bass || has_drums || has_guitar || has_strings ||
        has_synth || has_brass || has_sax) {
        std::vector<BlueprintInstrument> out;
        auto add = [&](bool cond, std::string_view name, std::string_view role,
                       int ch, int prog) {
            if (cond)
                out.push_back({std::string(name), std::string(role), ch, prog});
        };
        add(has_piano,   "piano",   "comping", 0, 1);
        add(has_bass,    "bass",    "bass",    1, 34);
        add(has_drums,   "drums",   "drum",    9, 1);
        add(has_guitar,  "guitar",  "rhythm",  2, 27);
        add(has_strings, "strings", "pad",     3, 49);
        add(has_synth,   "synth",   "pad",     4, 81);
        add(has_brass,   "brass",   "lead",    5, 57);
        add(has_sax,     "sax",     "lead",    6, 66);
        return out;
    }

    // Default: piano + bass + drums
    return {
        {"piano", "comping", 0, 1},
        {"bass",  "bass",    1, 34},
        {"drums", "drum",    9, 1},
    };
}

// ---------------------------------------------------------------------------
//  RuleBasedPromptInterpreter
// ---------------------------------------------------------------------------

class RuleBasedPromptInterpreter final : public IPromptInterpreter {
public:
    MusicBlueprint interpret(std::string_view prompt, Seed seed) const override {
        if (seed == 0)
            seed = fnv1a(prompt);

        auto lower = to_lower(prompt);

        // Input validation — reject literal-note requests
        if (has_literal_notes(lower)) {
            MusicBlueprint empty_bp{};
            empty_bp.seed = seed;
            return empty_bp;
        }

        MusicBlueprint bp{};
        bp.seed    = seed;
        bp.ppq     = 480;
        bp.bpm     = detect_bpm(lower);

        auto g = detect_genre(lower);
        bp.genres.assign(g.begin(), g.end());
        if (bp.genres.empty())
            bp.genres.push_back("pop");

        auto [key, sc]   = detect_key_and_scale(lower);
        bp.root_key       = key;
        bp.scale          = sc;

        bp.energy   = detect_energy(lower);
        bp.emotion  = emotion_from_energy(bp.energy);

        bp.sections     = build_sections(lower, bp.energy);
        bp.instruments  = detect_instruments(lower);

        return bp;
    }
};

} // anonymous namespace

std::unique_ptr<IPromptInterpreter> make_prompt_interpreter() {
    return std::make_unique<RuleBasedPromptInterpreter>();
}

} // namespace aimidi::theory
