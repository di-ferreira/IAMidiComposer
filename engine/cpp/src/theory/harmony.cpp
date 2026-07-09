// Implementation: Harmony Engine (skeleton II-V-I generator).
//
// TODO: integrate with the Musical Intelligence blueprint.
// TODO: use a seeded RNG (e.g. std::mt19937_64) seeded by HarmonyRequest.seed.
#include <aimidi/theory/IHarmonyEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>

#include <memory>
#include <vector>

namespace aimidi::theory {

namespace {

// Skeleton: produces 4 chords per bar (I-IV-V-I pattern). Not used in tests
// yet, but wiring is here.

class HarmonyEngine final : public IHarmonyEngine {
public:
    explicit HarmonyEngine(std::shared_ptr<IScaleProvider> scales)
        : scales_(std::move(scales)) {}

    std::vector<MidiEvent> generate(const HarmonyRequest& req) const override {
        (void)req;
        (void)scales_;
        return {};  // TODO: place II-V-I-style chords once blueprint arrives.
    }

private:
    std::shared_ptr<IScaleProvider> scales_;
};

} // namespace

std::unique_ptr<IHarmonyEngine> make_harmony_engine(
    std::shared_ptr<IScaleProvider> scales) {
    return std::make_unique<HarmonyEngine>(std::move(scales));
}

} // namespace aimidi::theory
