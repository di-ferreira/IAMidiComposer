#pragma once
#include <aimidi/theory/Types.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <memory>
#include <vector>
#include <string>
#include <string_view>

namespace aimidi::theory {

enum class OrchestralRole { melody, harmony, bass, pad, rhythm, countermelody, fill };

struct OrchestralInstrument {
    std::string name;
    int         gm_program;
    int         channel;
    int         min_note;
    int         max_note;
    OrchestralRole role;
    int         priority;
};

struct OrchestralSection {
    std::string name;
    std::vector<OrchestralInstrument> instruments;
};

class IOrchestrationEngine {
public:
    virtual ~IOrchestrationEngine() = default;

    virtual std::vector<std::pair<std::string, std::vector<MidiEvent>>> orchestrate_chords(
        const std::vector<std::string>& chord_progression,
        const std::vector<OrchestralSection>& ensemble,
        int bars, int bpm, Seed seed) const = 0;

    virtual std::vector<std::pair<std::string, std::vector<MidiEvent>>> orchestrate_full(
        const std::vector<MidiEvent>& melody,
        const std::vector<std::string>& chord_progression,
        const std::vector<OrchestralSection>& ensemble,
        int bars, int bpm, Seed seed) const = 0;

    virtual std::vector<OrchestralSection> suggest_ensemble(
        std::string_view genre, int instrument_count) const = 0;
};

std::unique_ptr<IOrchestrationEngine> make_orchestration_engine(
    std::shared_ptr<IScaleProvider> scales,
    std::shared_ptr<IChordEngine> chords);

} // namespace aimidi::theory
