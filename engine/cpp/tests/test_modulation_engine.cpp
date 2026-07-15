#include <aimidi/theory/IModulationEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace aimidi::theory;

TEST(ModulationEngineTest, CreatesEngine) {
    auto scales = make_scale_provider();
    auto engine = make_modulation_engine(std::move(scales));
    ASSERT_TRUE(engine);
}

TEST(ModulationEngineTest, PivotCtoG) {
    auto scales = make_scale_provider();
    auto engine = make_modulation_engine(std::move(scales));
    auto pivot = engine->find_pivot_chord("C", "major", "G", "major");
    EXPECT_FALSE(pivot.empty());
}

TEST(ModulationEngineTest, PivotCtoF) {
    auto scales = make_scale_provider();
    auto engine = make_modulation_engine(std::move(scales));
    auto pivot = engine->find_pivot_chord("C", "major", "F", "major");
    EXPECT_FALSE(pivot.empty());
}

TEST(ModulationEngineTest, PivotCtoAm) {
    auto scales = make_scale_provider();
    auto engine = make_modulation_engine(std::move(scales));
    auto pivot = engine->find_pivot_chord("C", "major", "A", "minor");
    EXPECT_FALSE(pivot.empty());
}

TEST(ModulationEngineTest, CloselyRelated) {
    auto scales = make_scale_provider();
    auto engine = make_modulation_engine(std::move(scales));
    EXPECT_TRUE(engine->are_closely_related("C", "major", "G", "major"));
    EXPECT_FALSE(engine->are_closely_related("C", "major", "F#", "major"));
}

TEST(ModulationEngineTest, GenerateModulation) {
    auto scales = make_scale_provider();
    auto engine = make_modulation_engine(std::move(scales));
    auto mod = engine->generate_modulation("C", "major", "G", "major", 2, 42);
    EXPECT_FALSE(mod.empty());
}

TEST(ModulationEngineTest, SeedDeterministic) {
    auto scales = make_scale_provider();
    auto engine = make_modulation_engine(std::move(scales));
    auto a = engine->generate_modulation("C", "major", "G", "major", 2, 42);
    auto b = engine->generate_modulation("C", "major", "G", "major", 2, 42);
    EXPECT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a[i].first, b[i].first);
        EXPECT_EQ(a[i].second, b[i].second);
    }
}
