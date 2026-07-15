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

TEST(PromptInterpreterTest, DetectsGenre) {
    auto interpreter = make_prompt_interpreter();
    auto bp = interpreter->interpret("jazz piano trio", 0);
    ASSERT_FALSE(bp.genres.empty());
    EXPECT_EQ(bp.genres[0], "jazz");
}

TEST(PromptInterpreterTest, DetectsKey) {
    auto interpreter = make_prompt_interpreter();
    auto bp = interpreter->interpret("song in D minor", 0);
    EXPECT_EQ(bp.root_key, "D");
    EXPECT_EQ(bp.scale, "minor");
}

TEST(PromptInterpreterTest, DetectsBPM) {
    auto interpreter = make_prompt_interpreter();
    auto bp = interpreter->interpret("140 bpm electronic track", 0);
    EXPECT_EQ(bp.bpm, 140);
}

TEST(PromptInterpreterTest, DetectsEnergy) {
    auto interpreter = make_prompt_interpreter();
    auto bp = interpreter->interpret("calm relaxing piano piece", 0);
    EXPECT_EQ(bp.energy, EnergyLevel::low);
}

TEST(PromptInterpreterTest, DetectsInstruments) {
    auto interpreter = make_prompt_interpreter();
    auto bp = interpreter->interpret("piano bass drums trio", 0);
    EXPECT_EQ(bp.instruments.size(), 3u);
}

TEST(PromptInterpreterTest, RejectsLiteralNotes) {
    auto interpreter = make_prompt_interpreter();
    auto bp = interpreter->interpret("play notes C E G", 0);
    EXPECT_TRUE(bp.sections.empty());
}

TEST(PromptInterpreterTest, HasFourSectionsDefault) {
    auto interpreter = make_prompt_interpreter();
    auto bp = interpreter->interpret("pop song", 0);
    EXPECT_EQ(bp.sections.size(), 4u);
}

TEST(PromptInterpreterTest, CustomStructure) {
    auto interpreter = make_prompt_interpreter();
    auto bp = interpreter->interpret("intro 4 bars verse 8 bars chorus 8 bars", 0);
    EXPECT_GE(bp.sections.size(), 2u);
}
