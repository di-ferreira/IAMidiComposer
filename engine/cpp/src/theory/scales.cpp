// Implementation: ScaleProvider (deterministic lookup of scales).
#include <aimidi/theory/IScaleProvider.hpp>
#include <unordered_map>
#include <string_view>
#include <array>

namespace aimidi::theory {

namespace {

constexpr std::array<int, 7> kMajor    = {0, 2, 4, 5, 7, 9, 11};
constexpr std::array<int, 7> kMinor    = {0, 2, 3, 5, 7, 8, 10};
constexpr std::array<int, 7> kHarmMinor= {0, 2, 3, 5, 7, 8, 11};
constexpr std::array<int, 7> kMelMinor = {0, 2, 3, 5, 7, 9, 11};
constexpr std::array<int, 5> kPentMaj  = {0, 2, 4, 7, 9};
constexpr std::array<int, 5> kPentMin  = {0, 3, 5, 7, 10};
constexpr std::array<int, 6> kBlues    = {0, 3, 5, 6, 7, 10};

const std::unordered_map<std::string_view, std::span<const int>> kScales = {
    {"major",           kMajor},
    {"minor",           kMinor},
    {"harmonic_minor",  kHarmMinor},
    {"melodic_minor",   kMelMinor},
    {"pentatonic_major",kPentMaj},
    {"pentatonic_minor",kPentMin},
    {"blues",           kBlues},
};

} // namespace

class ScaleProvider final : public IScaleProvider {
public:
    std::span<const int> intervals_of(std::string_view name) const override {
        const auto it = kScales.find(name);
        return it != kScales.end() ? it->second : std::span<const int>{};
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
