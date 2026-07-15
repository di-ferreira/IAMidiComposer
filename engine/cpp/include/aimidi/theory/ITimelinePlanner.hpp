#pragma once
#include <aimidi/theory/IPromptInterpreter.hpp>
#include <vector>
#include <memory>
#include <cstdint>

namespace aimidi::theory {

struct PlannedSection {
    std::string  name;
    int          start_bar = 0;
    int          end_bar   = 0;
    int          bars      = 0;
    EnergyLevel  energy    = EnergyLevel::mid;
    float        density   = 0.5f;
    float        complexity = 0.5f;
};

struct SectionTransition {
    int         from_section_index = 0;
    int         to_section_index   = 0;
    int         transition_bars    = 0;
    std::string type               = "direct";
};

struct Timeline {
    std::vector<PlannedSection>    sections;
    std::vector<SectionTransition> transitions;
    int                            total_bars  = 0;
    int                            total_ticks = 0;
};

class ITimelinePlanner {
public:
    virtual ~ITimelinePlanner() = default;
    virtual Timeline plan(const MusicBlueprint& blueprint) const = 0;
};

std::unique_ptr<ITimelinePlanner> make_timeline_planner();

} // namespace aimidi::theory
