#include <aimidi/theory/IStringsEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace aimidi::theory;

namespace {

std::unique_ptr<IStringsEngine> make_engine() {
    auto scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    auto chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    return make_strings_engine(std::move(scales), std::move(chords));
}

} // namespace

TEST(StringsEngineTest, CreatesEngine) {
    auto engine = make_engine();
    ASSERT_TRUE(engine);
}

TEST(StringsEngineTest, PadReturnsEvents) {
    auto engine = make_engine();
    auto events = engine->generate_pad("C", "major", {"C", "F", "G", "C"}, 2, 120, StringsSection::full_ensemble, 42);
    EXPECT_FALSE(events.empty());
}

TEST(StringsEngineTest, CountermelodyReturnsEvents) {
    auto engine = make_engine();
    auto events = engine->generate_countermelody("C", "major", {"C", "F", "G", "C"}, 2, 120, StringsSection::violin, 42);
    EXPECT_FALSE(events.empty());
}

TEST(StringsEngineTest, SwellsReturnsEvents) {
    auto engine = make_engine();
    auto events = engine->generate_swells("C", "major", {"C", "F", "G", "C"}, 2, 120, StringsSection::full_ensemble, 42);
    EXPECT_FALSE(events.empty());
}

TEST(StringsEngineTest, SeedDeterministic) {
    auto engine = make_engine();
    auto a = engine->generate_pad("C", "major", {"C", "F", "G", "C"}, 2, 120, StringsSection::full_ensemble, 42);
    auto b = engine->generate_pad("C", "major", {"C", "F", "G", "C"}, 2, 120, StringsSection::full_ensemble, 42);
    ASSERT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a[i].note, b[i].note);
        EXPECT_EQ(a[i].tick_on, b[i].tick_on);
    }
}

TEST(StringsEngineTest, ViolinRange) {
    auto engine = make_engine();
    auto events = engine->generate_pad("C", "major", {"C", "F", "G", "C"}, 2, 120, StringsSection::violin, 42);
    for (const auto& e : events) {
        EXPECT_GE(e.note, 55);
        EXPECT_LE(e.note, 84);
    }
}

TEST(StringsEngineTest, CelloRange) {
    auto engine = make_engine();
    auto events = engine->generate_pad("C", "major", {"C", "F", "G", "C"}, 2, 120, StringsSection::cello, 42);
    for (const auto& e : events) {
        EXPECT_GE(e.note, 36);
        EXPECT_LE(e.note, 60);
    }
}

TEST(StringsEngineTest, ViolaRange) {
    auto engine = make_engine();
    auto events = engine->generate_pad("C", "major", {"C", "F", "G", "C"}, 2, 120, StringsSection::viola, 42);
    for (const auto& e : events) {
        EXPECT_GE(e.note, 48);
        EXPECT_LE(e.note, 76);
    }
}

TEST(StringsEngineTest, ContrabassRange) {
    auto engine = make_engine();
    auto events = engine->generate_pad("C", "major", {"C", "F", "G", "C"}, 2, 120, StringsSection::contrabass, 42);
    for (const auto& e : events) {
        EXPECT_GE(e.note, 28);
        EXPECT_LE(e.note, 48);
    }
}

TEST(StringsEngineTest, CountermelodySeedDeterministic) {
    auto engine = make_engine();
    auto a = engine->generate_countermelody("C", "major", {"C", "F", "G", "C"}, 2, 120, StringsSection::violin, 42);
    auto b = engine->generate_countermelody("C", "major", {"C", "F", "G", "C"}, 2, 120, StringsSection::violin, 42);
    ASSERT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a[i].note, b[i].note);
        EXPECT_EQ(a[i].tick_on, b[i].tick_on);
    }
}

TEST(StringsEngineTest, SwellsSeedDeterministic) {
    auto engine = make_engine();
    auto a = engine->generate_swells("C", "major", {"C", "F", "G", "C"}, 2, 120, StringsSection::full_ensemble, 42);
    auto b = engine->generate_swells("C", "major", {"C", "F", "G", "C"}, 2, 120, StringsSection::full_ensemble, 42);
    ASSERT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a[i].note, b[i].note);
        EXPECT_EQ(a[i].tick_on, b[i].tick_on);
    }
}
