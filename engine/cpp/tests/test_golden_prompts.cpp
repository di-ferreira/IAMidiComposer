// Golden prompt dataset for regression testing.
//
// Each prompt should produce a deterministic MusicBlueprint (and downstream
// Timeline/Arrangement) given a fixed seed.  Verifies key, scale, bpm,
// minimum section/instrument counts, and seed reproducibility.
#include <aimidi/theory/IPromptInterpreter.hpp>
#include <aimidi/theory/IBlueprintGenerator.hpp>
#include <aimidi/theory/ITimelinePlanner.hpp>
#include <aimidi/theory/IArrangementPlanner.hpp>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

using namespace aimidi::theory;

namespace {

struct PromptTest {
    std::string name;
    std::string prompt;
    Seed        seed;
    std::string expected_key;
    std::string expected_scale;
    int         expected_bpm;
    int         min_sections;
    int         min_instruments;
};

const std::vector<PromptTest> kPromptTests = {
    {"pop_song",      "upbeat pop song in C major 120 bpm",       42, "C", "major", 120, 2, 2},
    {"jazz_trio",     "smooth jazz piano trio",                  7,  "C", "major", 120, 2, 2},
    {"rock_anthem",   "energetic rock anthem in E minor 140 bpm", 99, "E", "minor", 140, 2, 2},
    {"electronic",    "electronic dance track 128 bpm",           0,  "C", "major", 128, 2, 2},
    {"classical",     "calm classical piano piece in G major",    1,  "G", "major", 120, 2, 1},
    {"blues",         "slow blues in A",                          5,  "A", "major", 120, 2, 2},
    {"latin",         "upbeat latin salsa in D minor",            3,  "D", "minor", 120, 2, 2},
    {"hiphop",        "hip hop beat 95 bpm",                     10,  "C", "major", 95,  2, 2},
    {"folk",          "gentle folk song in G major",              8,  "G", "major", 120, 2, 2},
    {"funk",          "funky bass driven groove",                 6,  "C", "major", 120, 2, 2},
};

} // namespace

class GoldenPromptTest : public ::testing::TestWithParam<PromptTest> {};

TEST_P(GoldenPromptTest, DeterministicBlueprint) {
    const auto& param = GetParam();
    auto interpreter = make_prompt_interpreter();
    ASSERT_TRUE(interpreter);

    auto bp = interpreter->interpret(param.prompt, param.seed);
    EXPECT_EQ(bp.root_key, param.expected_key);
    EXPECT_EQ(bp.scale, param.expected_scale);
    EXPECT_EQ(bp.bpm, param.expected_bpm);
    EXPECT_GE(bp.sections.size(), static_cast<std::size_t>(param.min_sections));
    EXPECT_GE(bp.instruments.size(), static_cast<std::size_t>(param.min_instruments));
}

TEST_P(GoldenPromptTest, DeterministicTimeline) {
    const auto& param = GetParam();
    auto interpreter = make_prompt_interpreter();
    auto generator = make_blueprint_generator();
    auto planner = make_timeline_planner();
    ASSERT_TRUE(interpreter);
    ASSERT_TRUE(generator);
    ASSERT_TRUE(planner);

    auto bp = interpreter->interpret(param.prompt, param.seed);
    auto finalized = generator->generate(bp);
    auto tl = planner->plan(finalized);

    EXPECT_GT(tl.total_bars, 0);
    EXPECT_FALSE(tl.sections.empty());
}

TEST_P(GoldenPromptTest, SeedReproducible) {
    const auto& param = GetParam();
    auto interpreter = make_prompt_interpreter();

    auto a = interpreter->interpret(param.prompt, param.seed);
    auto b = interpreter->interpret(param.prompt, param.seed);

    EXPECT_EQ(a.root_key, b.root_key);
    EXPECT_EQ(a.scale, b.scale);
    EXPECT_EQ(a.bpm, b.bpm);
    EXPECT_EQ(a.sections.size(), b.sections.size());
    EXPECT_EQ(a.instruments.size(), b.instruments.size());
}

INSTANTIATE_TEST_SUITE_P(
    GoldenPrompts,
    GoldenPromptTest,
    ::testing::ValuesIn(kPromptTests),
    [](const ::testing::TestParamInfo<PromptTest>& param_info) {
        return param_info.param.name;
    }
);
