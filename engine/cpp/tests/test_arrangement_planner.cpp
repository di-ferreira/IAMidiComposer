#include <aimidi/theory/IArrangementPlanner.hpp>
#include <aimidi/theory/ITimelinePlanner.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace aimidi::theory;

TEST(ArrangementPlannerTest, ReturnsNonEmpty) {
    auto planner = make_arrangement_planner();
    ASSERT_TRUE(planner);
    MusicBlueprint bp{};
    bp.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    bp.instruments.push_back({"piano", "comping", 0, 1});
    bp.instruments.push_back({"bass", "bass", 1, 34});
    bp.instruments.push_back({"drums", "drum", 9, 1});
    auto tl_planner = make_timeline_planner();
    auto tl = tl_planner->plan(bp);
    auto arr = planner->plan(bp, tl);
    EXPECT_FALSE(arr.section_instruments.empty());
}

TEST(ArrangementPlannerTest, AllInstrumentsInHighEnergy) {
    auto planner = make_arrangement_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"chorus", 4, EnergyLevel::high, 0.8f, 0.6f});
    bp.instruments.push_back({"piano", "comping", 0, 1});
    bp.instruments.push_back({"bass", "bass", 1, 34});
    bp.instruments.push_back({"drums", "drum", 9, 1});
    bp.instruments.push_back({"lead_synth", "lead", 2, 81});
    auto tl_planner = make_timeline_planner();
    auto tl = tl_planner->plan(bp);
    auto arr = planner->plan(bp, tl);
    ASSERT_FALSE(arr.section_instruments.empty());
    EXPECT_GE(arr.section_instruments[0].active_instruments.size(), 3u);
}

TEST(ArrangementPlannerTest, FewerInstrumentsInLowEnergy) {
    auto planner = make_arrangement_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"intro", 2, EnergyLevel::low, 0.3f, 0.2f});
    bp.instruments.push_back({"piano", "comping", 0, 1});
    bp.instruments.push_back({"bass", "bass", 1, 34});
    bp.instruments.push_back({"drums", "drum", 9, 1});
    auto tl_planner = make_timeline_planner();
    auto tl = tl_planner->plan(bp);
    auto arr = planner->plan(bp, tl);
    ASSERT_FALSE(arr.section_instruments.empty());
    EXPECT_LE(arr.section_instruments[0].active_instruments.size(), 2u);
}

TEST(ArrangementPlannerTest, DrumsAlwaysPresent) {
    auto planner = make_arrangement_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"intro", 2, EnergyLevel::low, 0.3f, 0.2f});
    bp.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    bp.instruments.push_back({"piano", "comping", 0, 1});
    bp.instruments.push_back({"drums", "drum", 9, 1});
    auto tl_planner = make_timeline_planner();
    auto tl = tl_planner->plan(bp);
    auto arr = planner->plan(bp, tl);
    for (const auto& si : arr.section_instruments) {
        bool has_drums = false;
        for (const auto& inst : si.active_instruments) {
            if (inst.role == "drum") has_drums = true;
        }
        EXPECT_TRUE(has_drums) << "Drums missing in section " << si.section_name;
    }
}

TEST(ArrangementPlannerTest, AllInstrumentsListed) {
    auto planner = make_arrangement_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    bp.instruments.push_back({"piano", "comping", 0, 1});
    bp.instruments.push_back({"bass", "bass", 1, 34});
    auto tl_planner = make_timeline_planner();
    auto tl = tl_planner->plan(bp);
    auto arr = planner->plan(bp, tl);
    EXPECT_EQ(arr.all_instruments.size(), 2u);
}

TEST(ArrangementPlannerTest, SeedDeterministic) {
    auto planner = make_arrangement_planner();
    MusicBlueprint bp{};
    bp.seed = 42;
    bp.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    bp.instruments.push_back({"piano", "comping", 0, 1});
    auto tl_planner = make_timeline_planner();
    auto tl = tl_planner->plan(bp);
    auto a = planner->plan(bp, tl);
    auto b = planner->plan(bp, tl);
    EXPECT_EQ(a.section_instruments.size(), b.section_instruments.size());
}
