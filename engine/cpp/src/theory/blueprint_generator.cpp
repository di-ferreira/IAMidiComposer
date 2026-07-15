#include <aimidi/theory/IBlueprintGenerator.hpp>

#include <algorithm>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace aimidi::theory {

namespace {

// -----------------------------------------------------------------------
//  FNV-1a hash for deterministic seed generation
// -----------------------------------------------------------------------
constexpr std::uint64_t fnv1a(std::string_view s) noexcept {
    std::uint64_t hash = 14695981039346656037ULL;
    for (auto c : s) {
        hash ^= static_cast<std::uint8_t>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

// -----------------------------------------------------------------------
//  Validation tables
// -----------------------------------------------------------------------
const std::unordered_set<std::string_view> kValidKeys = {
    "C", "C#", "Db", "D", "D#", "Eb", "E",
    "F", "F#", "Gb", "G", "G#", "Ab", "A", "A#", "Bb", "B"
};

const std::unordered_set<std::string_view> kValidScales = {
    "major", "minor", "dorian", "phrygian", "lydian", "mixolydian",
    "aeolian", "locrian", "harmonic_minor", "melodic_minor",
    "pentatonic_major", "pentatonic_minor", "blues", "chromatic"
};

[[nodiscard]] bool is_valid_root(const std::string& key) {
    return kValidKeys.count(key) > 0;
}

[[nodiscard]] bool is_valid_scale(const std::string& scale) {
    return kValidScales.count(scale) > 0;
}

// -----------------------------------------------------------------------
//  Energy -> density / complexity mapping
// -----------------------------------------------------------------------
[[nodiscard]] float density_for_energy(EnergyLevel e) noexcept {
    switch (e) {
    case EnergyLevel::low:  return 0.3f;
    case EnergyLevel::mid:  return 0.5f;
    case EnergyLevel::high: return 0.8f;
    default:                return 0.5f;
    }
}

[[nodiscard]] float complexity_for_energy(EnergyLevel e) noexcept {
    switch (e) {
    case EnergyLevel::low:  return 0.2f;
    case EnergyLevel::mid:  return 0.4f;
    case EnergyLevel::high: return 0.6f;
    default:                return 0.4f;
    }
}

// -----------------------------------------------------------------------
//  Emotion from energy
// -----------------------------------------------------------------------
[[nodiscard]] Emotion emotion_from_energy(EnergyLevel e) noexcept {
    switch (e) {
    case EnergyLevel::low:
        return {.valence = 0.3f, .arousal = 0.2f, .dominance = 0.4f};
    case EnergyLevel::mid:
        return {.valence = 0.5f, .arousal = 0.5f, .dominance = 0.5f};
    case EnergyLevel::high:
        return {.valence = 0.8f, .arousal = 0.7f, .dominance = 0.6f};
    default:
        return {};
    }
}

[[nodiscard]] bool is_all_default_emotion(const Emotion& e) noexcept {
    return e.valence == 0.5f && e.arousal == 0.5f && e.dominance == 0.5f;
}

// -----------------------------------------------------------------------
//  Default section factory
// -----------------------------------------------------------------------
[[nodiscard]] std::vector<BlueprintSection> make_default_sections() {
    std::vector<BlueprintSection> sections;
    sections.reserve(4);
    sections.push_back({"intro",  2, EnergyLevel::unspecified, 0.5f, 0.5f});
    sections.push_back({"verse",  4, EnergyLevel::unspecified, 0.5f, 0.5f});
    sections.push_back({"chorus", 4, EnergyLevel::unspecified, 0.5f, 0.5f});
    sections.push_back({"outro",  2, EnergyLevel::unspecified, 0.5f, 0.5f});
    return sections;
}

// -----------------------------------------------------------------------
//  Default instrument factory
// -----------------------------------------------------------------------
[[nodiscard]] std::vector<BlueprintInstrument> make_default_instruments() {
    std::vector<BlueprintInstrument> instruments;
    instruments.reserve(3);
    instruments.push_back({"piano", "comping", 0, 1});    // channel 0
    instruments.push_back({"bass",  "bass",    1, 34});   // channel 1
    instruments.push_back({"drums", "drum",    9, 1});    // channel 9 (GM drums)
    return instruments;
}

// -----------------------------------------------------------------------
//  Concrete implementation
// -----------------------------------------------------------------------
class BlueprintGenerator final : public IBlueprintGenerator {
public:
    MusicBlueprint generate(const MusicBlueprint& input) const override {
        MusicBlueprint bp = input;

        // -- 1. Fill defaults -------------------------------------------
        if (bp.ppq == 0) {
            bp.ppq = 480;
        }

        if (bp.sections.empty()) {
            bp.sections = make_default_sections();
        }

        if (bp.instruments.empty()) {
            bp.instruments = make_default_instruments();
        }

        if (bp.seed == 0) {
            std::string seed_str = bp.root_key + bp.scale + std::to_string(bp.bpm);
            bp.seed = fnv1a(seed_str);
        }

        if (is_all_default_emotion(bp.emotion)) {
            bp.emotion = emotion_from_energy(bp.energy);
        }

        // -- 2. Validate -------------------------------------------------
        if (!is_valid_root(bp.root_key) || !is_valid_scale(bp.scale)) {
            return {};
        }
        if (bp.bpm < 40 || bp.bpm > 300) {
            return {};
        }
        if (bp.sections.empty() || bp.instruments.empty()) {
            return {};
        }
        for (const auto& sec : bp.sections) {
            if (sec.bars < 1 || sec.bars > 256) {
                return {};
            }
        }

        // -- 3. Normalize ------------------------------------------------

        // 3a. Section energies inherit from global if unspecified
        //     and set density/complexity from energy.
        for (auto& sec : bp.sections) {
            if (sec.energy == EnergyLevel::unspecified) {
                sec.energy = bp.energy;
            }
            sec.density    = density_for_energy(sec.energy);
            sec.complexity = complexity_for_energy(sec.energy);
        }

        // 3b. Channels: drums always on 9; others unique 0-9
        {
            std::vector<int> used;
            used.reserve(bp.instruments.size());
            for (auto& inst : bp.instruments) {
                if (inst.role == "drum") {
                    inst.channel = 9;
                }
                // Resolve conflicts with already-assigned channels
                auto it = std::find(used.begin(), used.end(), inst.channel);
                if (it != used.end()) {
                    // Assign first free channel in 0..9, skipping 9 for non-drums
                    int limit = (inst.role == "drum") ? 10 : 9;
                    for (int ch = 0; ch < limit; ++ch) {
                        if (std::find(used.begin(), used.end(), ch) == used.end()) {
                            inst.channel = ch;
                            break;
                        }
                    }
                }
                used.push_back(inst.channel);
            }
        }

        return bp;
    }
};

} // anonymous namespace

std::unique_ptr<IBlueprintGenerator> make_blueprint_generator() {
    return std::make_unique<BlueprintGenerator>();
}

} // namespace aimidi::theory
