// Implementation: ScaleProvider (deterministic lookup of scales + modes).
#include <aimidi/theory/IScaleProvider.hpp>
#include <unordered_map>
#include <string_view>
#include <array>
#include <vector>

namespace aimidi::theory {

namespace {

constexpr std::array<int, 7> kMajor    = {0, 2, 4, 5, 7, 9, 11};
constexpr std::array<int, 7> kMinor    = {0, 2, 3, 5, 7, 8, 10};
constexpr std::array<int, 7> kHarmMinor= {0, 2, 3, 5, 7, 8, 11};
constexpr std::array<int, 7> kMelMinor = {0, 2, 3, 5, 7, 9, 11};
constexpr std::array<int, 5> kPentMaj  = {0, 2, 4, 7, 9};
constexpr std::array<int, 5> kPentMin  = {0, 3, 5, 7, 10};
constexpr std::array<int, 6> kBlues    = {0, 3, 5, 6, 7, 10};

// ── Church modes (diatonic rotations of the major scale) ──────────
constexpr std::array<int, 7> kIonian    = {0, 2, 4, 5, 7, 9, 11};  // = major (I)
constexpr std::array<int, 7> kDorian    = {0, 2, 3, 5, 7, 9, 10};  // ii
constexpr std::array<int, 7> kPhrygian = {0, 1, 3, 5, 7, 8, 10};  // iii
constexpr std::array<int, 7> kLydian   = {0, 2, 4, 6, 7, 9, 11};  // IV
constexpr std::array<int, 7> kMixolydian={0, 2, 4, 5, 7, 9, 10};  // V
constexpr std::array<int, 7> kAeolian  = {0, 2, 3, 5, 7, 8, 10};  // = minor (vi)
constexpr std::array<int, 7> kLocrian  = {0, 1, 3, 5, 6, 8, 10};  // vii°

const std::unordered_map<std::string_view, std::span<const int>> kScales = {
    {"major",            kMajor},
    {"minor",            kMinor},
    {"harmonic_minor",   kHarmMinor},
    {"melodic_minor",    kMelMinor},
    {"pentatonic_major", kPentMaj},
    {"pentatonic_minor", kPentMin},
    {"blues",            kBlues},
    // ── mode aliases ──
    {"ionian",           kIonian},
    {"dorian",           kDorian},
    {"phrygian",         kPhrygian},
    {"lydian",           kLydian},
    {"mixolydian",       kMixolydian},
    {"aeolian",          kAeolian},
    {"locrian",          kLocrian},
};

} // namespace

class ScaleProvider final : public IScaleProvider {
public:
    std::span<const int> intervals_of(std::string_view name) const override {
        const auto it = kScales.find(name);
        return it != kScales.end() ? it->second : std::span<const int>{};
    }

    std::vector<int> intervals_for(std::string_view name, int root_pc) const override {
        const auto base = intervals_of(name);
        if (base.empty()) {
            return {};
        }
        std::vector<int> out;
        out.reserve(base.size());
        const int pc = ((root_pc % 12) + 12) % 12;
        for (int iv : base) {
            out.push_back((iv + pc) % 12);
        }
        return out;
    }

    bool knows(std::string_view name) const noexcept override {
        return kScales.contains(name);
    }
};

// Public factory: enables DI without exposing the class directly.
std::unique_ptr<IScaleProvider> make_scale_provider() {
    return std::make_unique<ScaleProvider>();
}

} // namespace aimidi::theory
