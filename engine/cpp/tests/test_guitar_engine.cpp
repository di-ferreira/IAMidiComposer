#include <aimidi/theory/IGuitarEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace aimidi::theory;

namespace {

std::unique_ptr<IGuitarEngine> make_engine() {
    auto scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    auto chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    return make_guitar_engine(std::move(scales), std::move(chords));
}

} // namespace

TEST(GuitarEngineTest, CreatesEngine) {
    auto engine = make_engine();
    ASSERT_TRUE(engine);
}

TEST(GuitarEngineTest, StrummingReturnsEvents) {
    auto engine = make_engine();
    auto events = engine->generate_chords("C", "major", {"C", "F", "G", "C"}, 2, 120, GuitarStyle::strumming, 42);
    EXPECT_FALSE(events.empty());
}

TEST(GuitarEngineTest, ArpeggioReturnsEvents) {
    auto engine = make_engine();
    auto events = engine->generate_chords("C", "major", {"C", "F", "G", "C"}, 2, 120, GuitarStyle::arpeggio, 42);
    EXPECT_FALSE(events.empty());
}

TEST(GuitarEngineTest, PowerChords) {
    auto engine = make_engine();
    auto events = engine->generate_power_chords({"C5", "G5", "F5"}, 2, 120, 42);
    EXPECT_FALSE(events.empty());
}

TEST(GuitarEngineTest, Riff) {
    auto engine = make_engine();
    auto events = engine->generate_riff("C", "major", 2, 120, 42);
    EXPECT_FALSE(events.empty());
}

TEST(GuitarEngineTest, FunkChops) {
    auto engine = make_engine();
    auto events = engine->generate_funk_chops({"C7", "F7", "G7"}, 2, 120, 42);
    EXPECT_FALSE(events.empty());
}

TEST(GuitarEngineTest, SeedDeterministic) {
    auto engine = make_engine();
    auto a = engine->generate_chords("C", "major", {"C", "F", "G", "C"}, 2, 120, GuitarStyle::strumming, 42);
    auto b = engine->generate_chords("C", "major", {"C", "F", "G", "C"}, 2, 120, GuitarStyle::strumming, 42);
    ASSERT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a[i].note, b[i].note);
        EXPECT_EQ(a[i].tick_on, b[i].tick_on);
    }
}

TEST(GuitarEngineTest, RangeCheck) {
    auto engine = make_engine();
    auto events = engine->generate_chords("C", "major", {"C", "F", "G", "C"}, 2, 120, GuitarStyle::strumming, 42);
    for (const auto& e : events) {
        EXPECT_GE(e.note, 40);
        EXPECT_LE(e.note, 84);
    }
}

TEST(GuitarEngineTest, EmptyProgressionReturnsEmpty) {
    auto engine = make_engine();
    auto events = engine->generate_chords("C", "major", {}, 2, 120, GuitarStyle::strumming, 42);
    EXPECT_TRUE(events.empty());
}

TEST(GuitarEngineTest, PowerChordsSeedDeterministic) {
    auto engine = make_engine();
    auto a = engine->generate_power_chords({"C5", "G5", "F5"}, 2, 120, 42);
    auto b = engine->generate_power_chords({"C5", "G5", "F5"}, 2, 120, 42);
    ASSERT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a[i].note, b[i].note);
    }
}

TEST(GuitarEngineTest, RiffSeedDeterministic) {
    auto engine = make_engine();
    auto a = engine->generate_riff("C", "major", 2, 120, 42);
    auto b = engine->generate_riff("C", "major", 2, 120, 42);
    ASSERT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a[i].note, b[i].note);
        EXPECT_EQ(a[i].tick_on, b[i].tick_on);
    }
}

TEST(GuitarEngineTest, FunkChopsSeedDeterministic) {
    auto engine = make_engine();
    auto a = engine->generate_funk_chops({"C7", "F7", "G7"}, 2, 120, 42);
    auto b = engine->generate_funk_chops({"C7", "F7", "G7"}, 2, 120, 42);
    ASSERT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a[i].note, b[i].note);
        EXPECT_EQ(a[i].tick_on, b[i].tick_on);
    }
}
