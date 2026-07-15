#include <aimidi/theory/IPromptInterpreter.hpp>

namespace aimidi::theory {

namespace {

class StubPromptInterpreter final : public IPromptInterpreter {
public:
    MusicBlueprint interpret(std::string_view prompt, Seed seed) const override {
        (void)prompt;

        MusicBlueprint bp{};
        bp.seed = seed;
        bp.root_key = "C";
        bp.scale = "major";
        bp.bpm = 120;
        bp.ppq = 480;
        bp.genres = {"pop"};
        bp.emotion = {0.7f, 0.6f, 0.5f};
        bp.energy = EnergyLevel::mid;

        bp.sections.push_back({"intro",  2, EnergyLevel::low,  0.3f, 0.2f});
        bp.sections.push_back({"verse",  4, EnergyLevel::mid,  0.5f, 0.4f});
        bp.sections.push_back({"chorus", 4, EnergyLevel::high, 0.8f, 0.6f});
        bp.sections.push_back({"outro",  2, EnergyLevel::low,  0.3f, 0.2f});

        bp.instruments.push_back({"piano", "comping", 0, 1});
        bp.instruments.push_back({"bass",  "bass",    1, 34});
        bp.instruments.push_back({"drums", "drum",    9, 1});

        return bp;
    }
};

} // anonymous namespace

std::unique_ptr<IPromptInterpreter> make_prompt_interpreter() {
    return std::make_unique<StubPromptInterpreter>();
}

} // namespace aimidi::theory
