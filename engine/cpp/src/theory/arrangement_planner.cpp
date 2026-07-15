#include <aimidi/theory/IArrangementPlanner.hpp>
#include <algorithm>
#include <vector>

namespace aimidi::theory {

namespace {

[[nodiscard]] int energy_to_int(EnergyLevel e) {
    return static_cast<int>(e);
}

// Role→energy mapping with special overrides.
// Drums (role "drum") always play. Bass drops out on LOW.
[[nodiscard]] bool role_plays_at_energy(std::string_view role, EnergyLevel e) {
    if (role == "drum") return true;
    if (role == "lead") return energy_to_int(e) >= energy_to_int(EnergyLevel::high);
    if (role == "bass" || role == "rhythm")
        return energy_to_int(e) >= energy_to_int(EnergyLevel::mid);
    // pad, comping, and unknown roles play at all levels
    return true;
}

[[nodiscard]] std::vector<int>
target_indices(const MusicBlueprint& bp, EnergyLevel e) {
    std::vector<int> out;
    for (int i = 0; i < static_cast<int>(bp.instruments.size()); ++i) {
        if (role_plays_at_energy(bp.instruments[i].role, e))
            out.push_back(i);
    }
    return out;
}

void add_one(std::vector<int>& active, const std::vector<int>& target) {
    for (int idx : target) {
        if (std::find(active.begin(), active.end(), idx) == active.end()) {
            active.push_back(idx);
            std::sort(active.begin(), active.end());
            return;
        }
    }
}

void remove_one(std::vector<int>& active, const std::vector<int>& target) {
    for (auto it = active.begin(); it != active.end(); ++it) {
        if (std::find(target.begin(), target.end(), *it) == target.end()) {
            active.erase(it);
            return;
        }
    }
}

} // anonymous namespace

class ArrangementPlanner final : public IArrangementPlanner {
public:
    Arrangement plan(const MusicBlueprint& blueprint,
                     const Timeline& timeline) const override {
        Arrangement arr;
        std::vector<int> active;

        for (int i = 0; i < static_cast<int>(timeline.sections.size()); ++i) {
            const auto& sec = timeline.sections[i];
            auto ideal = target_indices(blueprint, sec.energy);

            if (i == 0) {
                active = std::move(ideal);
            } else {
                int prev_e = energy_to_int(timeline.sections[i - 1].energy);
                int curr_e = energy_to_int(sec.energy);
                if (curr_e > prev_e) {
                    add_one(active, ideal);
                } else if (curr_e < prev_e) {
                    remove_one(active, ideal);
                } else {
                    active = std::move(ideal);
                }
            }

            SectionInstruments si;
            si.section_name  = sec.name;
            si.section_index = i;
            for (int idx : active)
                si.active_instruments.push_back(blueprint.instruments[idx]);
            arr.section_instruments.push_back(std::move(si));
        }

        arr.all_instruments = blueprint.instruments;
        return arr;
    }
};

std::unique_ptr<IArrangementPlanner> make_arrangement_planner() {
    return std::make_unique<ArrangementPlanner>();
}

} // namespace aimidi::theory
