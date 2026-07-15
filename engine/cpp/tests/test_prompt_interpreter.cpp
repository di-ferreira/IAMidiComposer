#include <aimidi/theory/IPromptInterpreter.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace aimidi::theory;

TEST(PromptInterpreterTest, ReturnsNonEmptyBlueprint) {
    auto interpreter = make_prompt_interpreter();
    ASSERT_TRUE(interpreter);
    auto bp = interpreter->interpret("upbeat pop song in C major", 42);
    EXPECT_EQ(bp.root_key, "C");
    EXPECT_EQ(bp.scale, "major");
    EXPECT_EQ(bp.bpm, 120);
    EXPECT_FALSE(bp.sections.empty());
    EXPECT_FALSE(bp.instruments.empty());
}

TEST(PromptInterpreterTest, SeedDeterministic) {
    auto interpreter = make_prompt_interpreter();
    auto a = interpreter->interpret("test prompt", 123);
    auto b = interpreter->interpret("test prompt", 123);
    EXPECT_EQ(a.seed, b.seed);
    EXPECT_EQ(a.root_key, b.root_key);
    EXPECT_EQ(a.sections.size(), b.sections.size());
    EXPECT_EQ(a.instruments.size(), b.instruments.size());
}

TEST(PromptInterpreterTest, DefaultBlueprintHasPopGenre) {
    auto interpreter = make_prompt_interpreter();
    auto bp = interpreter->interpret("anything", 0);
    ASSERT_FALSE(bp.genres.empty());
    EXPECT_EQ(bp.genres[0], "pop");
}

TEST(PromptInterpreterTest, HasFourSections) {
    auto interpreter = make_prompt_interpreter();
    auto bp = interpreter->interpret("test", 0);
    EXPECT_EQ(bp.sections.size(), 4u);
}

TEST(PromptInterpreterTest, HasThreeInstruments) {
    auto interpreter = make_prompt_interpreter();
    auto bp = interpreter->interpret("test", 0);
    EXPECT_EQ(bp.instruments.size(), 3u);
}
