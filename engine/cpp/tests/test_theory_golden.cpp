// Test: theory/golden — byte-stable determinism across runs (Phase 2.1).
//
// Golden MIDI test at the std::vector<MidiEvent> level: same seed → identical
// bytes. Full SMF serialization is Sprint 6; this gates determinism early.
#include <aimidi/theory/IHarmonyEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>

#include <gtest/gtest.h>

#include <memory>

using namespace aimidi::theory;

namespace {
std::unique_ptr<IHarmonyEngine> make_engine() {
    auto scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    auto chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    return make_harmony_engine(std::move(scales), std::move(chords));
}
} // namespace

TEST(GoldenMidiTest, DeterministicSeed42) {
    auto engine = make_engine();
    const HarmonyRequest req{.root_key = "C", .scale = "major", .bars = 4, .seed = 42};
    const auto a = engine->generate(req);
    const auto b = engine->generate(req);
    EXPECT_EQ(a, b);
}

TEST(GoldenMidiTest, DeterministicSeed7) {
    auto engine = make_engine();
    const HarmonyRequest req{.root_key = "C", .scale = "major", .bars = 4, .seed = 7};
    const auto a = engine->generate(req);
    const auto b = engine->generate(req);
    EXPECT_EQ(a, b);
}

TEST(GoldenMidiTest, DeterministicSeed12345) {
    auto engine = make_engine();
    const HarmonyRequest req{.root_key = "C", .scale = "major", .bars = 4, .seed = 12345};
    const auto a = engine->generate(req);
    const auto b = engine->generate(req);
    EXPECT_EQ(a, b);
}

TEST(GoldenMidiTest, DeterministicSeed0) {
    auto engine = make_engine();
    const HarmonyRequest req{.root_key = "C", .scale = "major", .bars = 4, .seed = 0};
    const auto a = engine->generate(req);
    const auto b = engine->generate(req);
    EXPECT_EQ(a, b);
}

TEST(GoldenMidiTest, DeterministicSeed99) {
    auto engine = make_engine();
    const HarmonyRequest req{.root_key = "C", .scale = "major", .bars = 4, .seed = 99};
    const auto a = engine->generate(req);
    const auto b = engine->generate(req);
    EXPECT_EQ(a, b);
}
