#include <aimidi/theory/ITimelinePlanner.hpp>

#include <string>
#include <vector>

namespace aimidi::theory {

namespace {

[[nodiscard]] int energy_to_int(EnergyLevel e) {
    return static_cast<int>(e);
}

[[nodiscard]] bool has_energy_increased(const BlueprintSection& prev,
                                        const BlueprintSection& next) {
    return energy_to_int(next.energy) > energy_to_int(prev.energy);
}

[[nodiscard]] SectionTransition make_transition(int from, int to,
                                                int bars,
                                                std::string_view type) {
    return SectionTransition{
        .from_section_index = from,
        .to_section_index   = to,
        .transition_bars    = bars,
        .type               = std::string(type),
    };
}

[[nodiscard]] float density_for_energy(EnergyLevel e) {
    switch (e) {
    case EnergyLevel::low:  return 0.3f;
    case EnergyLevel::high: return 0.8f;
    default:                return 0.5f;
    }
}

[[nodiscard]] float complexity_for_energy(EnergyLevel e) {
    switch (e) {
    case EnergyLevel::low:  return 0.2f;
    case EnergyLevel::high: return 0.6f;
    default:                return 0.4f;
    }
}

PlannedSection make_planned(const BlueprintSection& s, int start_bar) {
    float dens = (s.density > 0.0f) ? s.density : density_for_energy(s.energy);
    float comp = (s.complexity > 0.0f) ? s.complexity : complexity_for_energy(s.energy);
    int   bars = (s.bars > 0) ? s.bars : 4;
    return PlannedSection{
        .name       = s.name,
        .start_bar  = start_bar,
        .end_bar    = start_bar + bars,
        .bars       = bars,
        .energy     = s.energy,
        .density    = dens,
        .complexity = comp,
    };
}

} // anonymous namespace

// -----------------------------------------------------------------------
//  TimelinePlanner implementation
// -----------------------------------------------------------------------

class TimelinePlanner final : public ITimelinePlanner {
public:
    Timeline plan(const MusicBlueprint& blueprint) const override {
        Timeline tl;

        // Build sections and transitions in a single pass so that
        // fill-transition bars shift subsequent section positions.
        int current_bar = 0;
        for (std::size_t i = 0; i < blueprint.sections.size(); ++i) {
            auto ps = make_planned(blueprint.sections[i], current_bar);
            tl.sections.push_back(ps);
            current_bar = ps.end_bar;

            if (i + 1 < blueprint.sections.size()) {
                auto trans = determine_transition(
                    static_cast<int>(i),
                    blueprint.sections[i],
                    blueprint.sections[i + 1]);
                current_bar += trans.transition_bars;
                tl.transitions.push_back(std::move(trans));
            }
        }

        tl.total_bars  = current_bar;
        tl.total_ticks = tl.total_bars * 4 * kPpq;

        return tl;
    }

private:
    [[nodiscard]] static SectionTransition
    determine_transition(int from_idx,
                         const BlueprintSection& prev,
                         const BlueprintSection& next) {
        // --- Named-rule transitions (override general rules) ---

        // To outro: always direct
        if (next.name == "outro") {
            return make_transition(from_idx, from_idx + 1, 0, "direct");
        }

        // intro -> verse: always direct
        if (prev.name == "intro" && next.name == "verse") {
            return make_transition(from_idx, from_idx + 1, 0, "direct");
        }

        // chorus -> verse: direct
        if (prev.name == "chorus" && next.name == "verse") {
            return make_transition(from_idx, from_idx + 1, 0, "direct");
        }

        // chorus -> bridge: fill
        if (prev.name == "chorus" && next.name == "bridge") {
            return make_transition(from_idx, from_idx + 1, 1, "fill");
        }

        // bridge -> chorus: fill
        if (prev.name == "bridge" && next.name == "chorus") {
            return make_transition(from_idx, from_idx + 1, 1, "fill");
        }

        // verse -> chorus: fill if energy increases
        if (prev.name == "verse" && next.name == "chorus") {
            if (has_energy_increased(prev, next)) {
                return make_transition(from_idx, from_idx + 1, 1, "fill");
            }
            return make_transition(from_idx, from_idx + 1, 0, "direct");
        }

        // --- General rules (energy-based) ---
        if (energy_to_int(prev.energy) != energy_to_int(next.energy)) {
            return make_transition(from_idx, from_idx + 1, 1, "fill");
        }

        // Same energy → direct
        return make_transition(from_idx, from_idx + 1, 0, "direct");
    }
};

std::unique_ptr<ITimelinePlanner> make_timeline_planner() {
    return std::make_unique<TimelinePlanner>();
}

} // namespace aimidi::theory
