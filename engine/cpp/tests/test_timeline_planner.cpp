#include <aimidi/theory/ITimelinePlanner.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace aimidi::theory;

TEST(TimelinePlannerTest, ReturnsNonEmpty) {
    auto planner = make_timeline_planner();
    ASSERT_TRUE(planner);
    MusicBlueprint bp{};
    bp.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    auto tl = planner->plan(bp);
    EXPECT_FALSE(tl.sections.empty());
}

TEST(TimelinePlannerTest, CorrectBarRanges) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"intro", 2, EnergyLevel::low, 0.3f, 0.2f});
    bp.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    bp.sections.push_back({"chorus", 4, EnergyLevel::high, 0.8f, 0.6f});
    auto tl = planner->plan(bp);
    ASSERT_EQ(tl.sections.size(), 3u);
    EXPECT_EQ(tl.sections[0].start_bar, 0);
    EXPECT_EQ(tl.sections[0].end_bar, 2);
    EXPECT_EQ(tl.sections[1].start_bar, 2);
    EXPECT_EQ(tl.sections[1].end_bar, 6);

    // chorus shifted by 1-bar fill transition (verse→chorus energy increase)
    EXPECT_EQ(tl.sections[2].start_bar, 7);
    EXPECT_EQ(tl.sections[2].end_bar, 11);

    // total = intro(2) + verse(4) + fill(1) + chorus(4) = 11
    EXPECT_EQ(tl.total_bars, 11);
}

TEST(TimelinePlannerTest, TotalTicks) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    auto tl = planner->plan(bp);
    EXPECT_EQ(tl.total_ticks, 4 * 4 * kPpq);
}

TEST(TimelinePlannerTest, AddsTransitions) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    bp.sections.push_back({"chorus", 4, EnergyLevel::high, 0.8f, 0.6f});
    auto tl = planner->plan(bp);
    EXPECT_FALSE(tl.transitions.empty());
}

TEST(TimelinePlannerTest, SeedDeterministic) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.seed = 42;
    bp.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    auto a = planner->plan(bp);
    auto b = planner->plan(bp);
    EXPECT_EQ(a.sections.size(), b.sections.size());
    EXPECT_EQ(a.total_bars, b.total_bars);
}

TEST(TimelinePlannerTest, InheritsDensityFromEnergy) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"verse", 4, EnergyLevel::high, 0.0f, 0.0f});
    auto tl = planner->plan(bp);
    EXPECT_FLOAT_EQ(tl.sections[0].density, 0.8f);
    EXPECT_FLOAT_EQ(tl.sections[0].complexity, 0.6f);
}

TEST(TimelinePlannerTest, IntroToVerseDirect) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"intro", 2, EnergyLevel::low, 0.3f, 0.2f});
    bp.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    auto tl = planner->plan(bp);
    ASSERT_EQ(tl.transitions.size(), 1u);
    EXPECT_EQ(tl.transitions[0].type, "direct");
    EXPECT_EQ(tl.transitions[0].transition_bars, 0);
}

TEST(TimelinePlannerTest, VerseToChorusFill) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    bp.sections.push_back({"chorus", 4, EnergyLevel::high, 0.8f, 0.6f});
    auto tl = planner->plan(bp);
    ASSERT_EQ(tl.transitions.size(), 1u);
    EXPECT_EQ(tl.transitions[0].type, "fill");
    EXPECT_EQ(tl.transitions[0].transition_bars, 1);
}

TEST(TimelinePlannerTest, ChorusToVerseDirect) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"chorus", 4, EnergyLevel::high, 0.8f, 0.6f});
    bp.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    auto tl = planner->plan(bp);
    ASSERT_EQ(tl.transitions.size(), 1u);
    EXPECT_EQ(tl.transitions[0].type, "direct");
    EXPECT_EQ(tl.transitions[0].transition_bars, 0);
}

TEST(TimelinePlannerTest, ChorusToBridgeFill) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"chorus", 4, EnergyLevel::high, 0.8f, 0.6f});
    bp.sections.push_back({"bridge", 4, EnergyLevel::mid, 0.5f, 0.4f});
    auto tl = planner->plan(bp);
    ASSERT_EQ(tl.transitions.size(), 1u);
    EXPECT_EQ(tl.transitions[0].type, "fill");
    EXPECT_EQ(tl.transitions[0].transition_bars, 1);
}

TEST(TimelinePlannerTest, BridgeToChorusFill) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"bridge", 4, EnergyLevel::mid, 0.5f, 0.4f});
    bp.sections.push_back({"chorus", 4, EnergyLevel::high, 0.8f, 0.6f});
    auto tl = planner->plan(bp);
    ASSERT_EQ(tl.transitions.size(), 1u);
    EXPECT_EQ(tl.transitions[0].type, "fill");
    EXPECT_EQ(tl.transitions[0].transition_bars, 1);
}

TEST(TimelinePlannerTest, ToOutroDirect) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"chorus", 4, EnergyLevel::high, 0.8f, 0.6f});
    bp.sections.push_back({"outro", 2, EnergyLevel::low, 0.3f, 0.2f});
    auto tl = planner->plan(bp);
    ASSERT_EQ(tl.transitions.size(), 1u);
    EXPECT_EQ(tl.transitions[0].type, "direct");
    EXPECT_EQ(tl.transitions[0].transition_bars, 0);
}

TEST(TimelinePlannerTest, SameEnergyDirect) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"verse_a", 4, EnergyLevel::mid, 0.5f, 0.4f});
    bp.sections.push_back({"verse_b", 4, EnergyLevel::mid, 0.5f, 0.4f});
    auto tl = planner->plan(bp);
    ASSERT_EQ(tl.transitions.size(), 1u);
    EXPECT_EQ(tl.transitions[0].type, "direct");
    EXPECT_EQ(tl.transitions[0].transition_bars, 0);
}

TEST(TimelinePlannerTest, TotalBarsIncludesTransitionBars) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    bp.sections.push_back({"chorus", 4, EnergyLevel::high, 0.8f, 0.6f});
    auto tl = planner->plan(bp);
    // 4 (verse) + 1 (fill transition) + 4 (chorus) = 9
    EXPECT_EQ(tl.total_bars, 9);
}

TEST(TimelinePlannerTest, DefaultBarsWhenZero) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"verse", 0, EnergyLevel::mid, 0.5f, 0.4f});
    auto tl = planner->plan(bp);
    ASSERT_EQ(tl.sections.size(), 1u);
    EXPECT_EQ(tl.sections[0].bars, 4);
    EXPECT_EQ(tl.sections[0].end_bar, 4);
}

TEST(TimelinePlannerTest, InheritsDensityFromEnergyLow) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"intro", 2, EnergyLevel::low, 0.0f, 0.0f});
    auto tl = planner->plan(bp);
    EXPECT_FLOAT_EQ(tl.sections[0].density, 0.3f);
    EXPECT_FLOAT_EQ(tl.sections[0].complexity, 0.2f);
}

TEST(TimelinePlannerTest, TotalTicksLargeProject) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"intro",  2, EnergyLevel::low,  0.3f, 0.2f});
    bp.sections.push_back({"verse",  8, EnergyLevel::mid,  0.5f, 0.4f});
    bp.sections.push_back({"chorus", 8, EnergyLevel::high, 0.8f, 0.6f});
    bp.sections.push_back({"bridge", 4, EnergyLevel::mid,  0.5f, 0.4f});
    bp.sections.push_back({"chorus", 8, EnergyLevel::high, 0.8f, 0.6f});
    bp.sections.push_back({"outro",  2, EnergyLevel::low,  0.3f, 0.2f});
    auto tl = planner->plan(bp);
    // intro(2) + direct + verse(8) + fill(1) + chorus(8) + fill(1) + bridge(4)
    //   + fill(1) + chorus(8) + direct + outro(2) = 35
    EXPECT_EQ(tl.total_bars, 35);
    EXPECT_EQ(tl.total_ticks, 35 * 4 * kPpq);
}

TEST(TimelinePlannerTest, KeepsGivenDensity) {
    auto planner = make_timeline_planner();
    MusicBlueprint bp{};
    bp.sections.push_back({"verse", 4, EnergyLevel::low, 0.7f, 0.3f});
    auto tl = planner->plan(bp);
    EXPECT_FLOAT_EQ(tl.sections[0].density, 0.7f);
    EXPECT_FLOAT_EQ(tl.sections[0].complexity, 0.3f);
}
