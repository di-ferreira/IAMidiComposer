#pragma once
#include <aimidi/theory/IPromptInterpreter.hpp>
#include <aimidi/theory/ITimelinePlanner.hpp>
#include <vector>
#include <string>
#include <memory>

namespace aimidi::theory {

struct SectionInstruments {
    std::string                          section_name;
    int                                  section_index;
    std::vector<BlueprintInstrument>     active_instruments;
};

struct Arrangement {
    std::vector<SectionInstruments> section_instruments;
    std::vector<BlueprintInstrument> all_instruments;
};

class IArrangementPlanner {
public:
    virtual ~IArrangementPlanner() = default;
    virtual Arrangement plan(const MusicBlueprint& blueprint, const Timeline& timeline) const = 0;
};

std::unique_ptr<IArrangementPlanner> make_arrangement_planner();

} // namespace aimidi::theory
