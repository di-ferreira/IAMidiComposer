#include <aimidi/theory/ICounterpointEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace aimidi::theory;

TEST(CounterpointEngineTest, CreatesEngine) {
    auto scales = make_scale_provider();
    auto engine = make_counterpoint_engine(std::move(scales));
    ASSERT_TRUE(engine);
}

TEST(CounterpointEngineTest, FirstSpeciesReturnsNonEmpty) {
    auto scales = make_scale_provider();
    auto engine = make_counterpoint_engine(std::move(scales));
    CounterpointRequest req{};
    req.cantus_firmus = {0, 2, 4, 5, 7, 9, 11, 0};
    req.bass_notes = {0, 2, 4, 5, 7, 9, 11, 0};
    req.bars = 2;
    req.seed = 42;
    req.species = CounterpointSpecies::first;
    auto result = engine->generate(req);
    EXPECT_FALSE(result.empty());
}

TEST(CounterpointEngineTest, SecondSpeciesReturnsNonEmpty) {
    auto scales = make_scale_provider();
    auto engine = make_counterpoint_engine(std::move(scales));
    CounterpointRequest req{};
    req.cantus_firmus = {0, 2, 4, 5, 7};
    req.bass_notes = {0, 2, 4, 5, 7};
    req.bars = 2;
    req.seed = 42;
    req.species = CounterpointSpecies::second;
    auto result = engine->generate(req);
    EXPECT_FALSE(result.empty());
}

TEST(CounterpointEngineTest, SeedDeterministic) {
    auto scales = make_scale_provider();
    auto engine = make_counterpoint_engine(std::move(scales));
    CounterpointRequest req{};
    req.cantus_firmus = {0, 4, 7};
    req.bass_notes = {0, 4, 7};
    req.bars = 1;
    req.seed = 42;
    req.species = CounterpointSpecies::first;
    auto a = engine->generate(req);
    auto b = engine->generate(req);
    EXPECT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a[i].note, b[i].note);
    }
}

TEST(CounterpointEngineTest, NoParallelFifths) {
    auto scales = make_scale_provider();
    auto engine = make_counterpoint_engine(std::move(scales));
    CounterpointRequest req{};
    req.cantus_firmus = {0, 2, 4, 5, 7, 9, 11, 0};
    req.bass_notes = {0, 2, 4, 5, 7, 9, 11, 0};
    req.bars = 2;
    req.seed = 42;
    req.species = CounterpointSpecies::first;
    auto result = engine->generate(req);
    for (std::size_t i = 1; i < result.size(); ++i) {
        int prev_interval = (result[i-1].note - req.bass_notes[i-1]) % 12;
        int curr_interval = (result[i].note - req.bass_notes[i]) % 12;
        EXPECT_FALSE(prev_interval == 7 && curr_interval == 7);
        EXPECT_FALSE(prev_interval == 0 && curr_interval == 0);
    }
}

TEST(CounterpointEngineTest, DurationPositive) {
    auto scales = make_scale_provider();
    auto engine = make_counterpoint_engine(std::move(scales));
    CounterpointRequest req{};
    req.cantus_firmus = {0, 2, 4};
    req.bass_notes = {0, 2, 4};
    req.bars = 1;
    req.seed = 7;
    req.species = CounterpointSpecies::first;
    auto result = engine->generate(req);
    for (const auto& e : result) {
        EXPECT_GT(e.tick_off, e.tick_on);
    }
}
