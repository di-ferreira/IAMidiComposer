#include <aimidi/theory/IBlueprintGenerator.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace aimidi::theory;

TEST(BlueprintGeneratorTest, ReturnsNonEmpty) {
    auto gen = make_blueprint_generator();
    ASSERT_TRUE(gen);
    MusicBlueprint input{};
    input.root_key = "C";
    input.scale = "major";
    input.bpm = 120;
    input.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    input.instruments.push_back({"piano", "comping", 0, 1});
    auto bp = gen->generate(input);
    EXPECT_FALSE(bp.sections.empty());
    EXPECT_FALSE(bp.instruments.empty());
}

TEST(BlueprintGeneratorTest, FillsDefaultSections) {
    auto gen = make_blueprint_generator();
    MusicBlueprint input{};
    input.root_key = "C";
    input.scale = "major";
    input.bpm = 120;
    input.instruments.push_back({"piano", "comping", 0, 1});
    auto bp = gen->generate(input);
    EXPECT_FALSE(bp.sections.empty());
    EXPECT_GE(bp.sections.size(), 2u);
}

TEST(BlueprintGeneratorTest, FillsDefaultInstruments) {
    auto gen = make_blueprint_generator();
    MusicBlueprint input{};
    input.root_key = "C";
    input.scale = "major";
    input.bpm = 120;
    input.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    auto bp = gen->generate(input);
    EXPECT_FALSE(bp.instruments.empty());
}

TEST(BlueprintGeneratorTest, NormalizesEnergy) {
    auto gen = make_blueprint_generator();
    MusicBlueprint input{};
    input.root_key = "C";
    input.scale = "major";
    input.bpm = 120;
    input.energy = EnergyLevel::high;
    input.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    input.instruments.push_back({"piano", "comping", 0, 1});
    auto bp = gen->generate(input);
    EXPECT_GT(bp.emotion.valence, 0.5f);
}

TEST(BlueprintGeneratorTest, DrumsOnChannel9) {
    auto gen = make_blueprint_generator();
    MusicBlueprint input{};
    input.root_key = "C";
    input.scale = "major";
    input.bpm = 120;
    input.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    input.instruments.push_back({"drums", "drum", 0, 1});  // wrong channel
    auto bp = gen->generate(input);
    for (const auto& inst : bp.instruments) {
        if (inst.role == "drum") {
            EXPECT_EQ(inst.channel, 9);
        }
    }
}

TEST(BlueprintGeneratorTest, SeedDeterministic) {
    auto gen = make_blueprint_generator();
    MusicBlueprint input{};
    input.seed = 42;
    input.root_key = "C";
    input.scale = "major";
    input.bpm = 120;
    input.sections.push_back({"verse", 4, EnergyLevel::mid, 0.5f, 0.4f});
    input.instruments.push_back({"piano", "comping", 0, 1});
    auto a = gen->generate(input);
    auto b = gen->generate(input);
    EXPECT_EQ(a.sections.size(), b.sections.size());
    EXPECT_EQ(a.instruments.size(), b.instruments.size());
}
