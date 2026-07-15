#pragma once
#include <aimidi/theory/Types.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IOrchestrationEngine.hpp>
#include <aimidi/theory/IStringsEngine.hpp>
#include <aimidi/theory/IGuitarEngine.hpp>
#include <aimidi/theory/IBassEngine.hpp>
#include <aimidi/theory/IDrumEngine.hpp>
#include <memory>
#include <vector>
#include <string>
#include <string_view>

namespace aimidi::theory {

// Orchestration density.
enum class OrchestrateDensity { sparse, medium, full };

// OrchestrateRequest: piano idea → orchestral arrangement.
struct OrchestrateRequest {
    std::vector<MidiEvent> piano_input;
    std::string_view       key;
    std::string_view       scale;
    int                    bars = 4;
    int                    bpm  = 120;
    OrchestrateDensity     density = OrchestrateDensity::medium;
    Seed                   seed = 0;
};

// OrchestrateWorkflow: W9 implementation.
class IOrchestrateWorkflow {
public:
    virtual ~IOrchestrateWorkflow() = default;

    // Transform piano idea into full orchestral arrangement.
    // Returns map of instrument name -> MIDI events.
    virtual std::vector<std::pair<std::string, std::vector<MidiEvent>>> orchestrate(
        const OrchestrateRequest& req) const = 0;
};

std::unique_ptr<IOrchestrateWorkflow> make_orchestrate_workflow(
    std::shared_ptr<IOrchestrationEngine> orch,
    std::shared_ptr<IStringsEngine> strings,
    std::shared_ptr<IGuitarEngine> guitar,
    std::shared_ptr<IBassEngine> bass,
    std::shared_ptr<IDrumEngine> drums);

} // namespace aimidi::theory
