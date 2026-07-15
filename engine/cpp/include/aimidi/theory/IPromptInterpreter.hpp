#pragma once
#include <aimidi/theory/Types.hpp>
#include <string>
#include <string_view>
#include <vector>
#include <memory>
#include <cstdint>

namespace aimidi::theory {

enum class EnergyLevel { unspecified = 0, low = 1, mid = 2, high = 3 };

struct Emotion {
    float valence   = 0.5f;
    float arousal   = 0.5f;
    float dominance = 0.5f;
};

struct BlueprintSection {
    std::string name;
    int         bars = 4;
    EnergyLevel energy = EnergyLevel::mid;
    float       density = 0.5f;
    float       complexity = 0.5f;
};

struct BlueprintInstrument {
    std::string name;
    std::string role;
    int         channel = 0;
    int         program = 0;
};

struct MusicBlueprint {
    Seed                            seed = 0;
    std::string                     root_key = "C";
    std::string                     scale = "major";
    int                             bpm = 120;
    int                             ppq = 480;
    std::vector<std::string>        genres;
    Emotion                         emotion;
    EnergyLevel                     energy = EnergyLevel::mid;
    std::vector<BlueprintSection>   sections;
    std::vector<BlueprintInstrument> instruments;
};

class IPromptInterpreter {
public:
    virtual ~IPromptInterpreter() = default;
    virtual MusicBlueprint interpret(std::string_view prompt, Seed seed = 0) const = 0;
};

std::unique_ptr<IPromptInterpreter> make_prompt_interpreter();

} // namespace aimidi::theory
