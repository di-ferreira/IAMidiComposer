// Test: theory/harmony — Phase 2.1 II-V-I generator (MIDI spike).
//
// Validates that HarmonyEngine now emits real MIDI for valid requests and that
// the II-V-I pattern per 4 bars is honored, plus seed reproducibility.
#include <aimidi/theory/IHarmonyEngine.hpp>
#include <aimidi/theory/IScaleProvider.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <aimidi/core/ServiceLocator.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <vector>
#include <algorithm>
#include <set>
#include <cstdint>

using namespace aimidi::theory;

namespace {
std::unique_ptr<IHarmonyEngine> make_full_engine() {
    auto scales = std::shared_ptr<IScaleProvider>(make_scale_provider().release());
    auto chords = std::shared_ptr<IChordEngine>(make_chord_engine().release());
    return make_harmony_engine(std::move(scales), std::move(chords));
}
} // namespace

TEST(HarmonyEngineTest, ValidRequestProducesNonEmptyMidi) {
    auto engine = make_full_engine();
    ASSERT_TRUE(engine);
    const HarmonyRequest req{.root_key = "C", .scale = "major", .bars = 4, .seed = 42};
    const auto events = engine->generate(req);
    EXPECT_FALSE(events.empty());
    // Each bar contributes >= 3 notes (triads or more).
    EXPECT_GE(events.size(), 12u);
    const std::uint32_t ticks_per_bar = static_cast<std::uint32_t>(kPpq) * 4u;
    // verify every event's tick_on aligns to a bar boundary.
    for (const auto& e : events) {
        EXPECT_EQ(e.tick_on % ticks_per_bar, 0u)
            << "tick_on=" << e.tick_on << " not aligned to bar grid";
    }
}

TEST(HarmonyEngineTest, SeedReproducibility) {
    auto engine = make_full_engine();
    const HarmonyRequest req{.root_key = "C", .scale = "major", .bars = 4, .seed = 42};
    const auto a = engine->generate(req);
    const auto b = engine->generate(req);
    EXPECT_EQ(a, b);
}

TEST(HarmonyEngineTest, IIVIPatternBars) {
    auto engine = make_full_engine();
    const HarmonyRequest req{.root_key = "C", .scale = "major", .bars = 4, .seed = 42};
    const auto events = engine->generate(req);
    ASSERT_FALSE(events.empty());
    const std::uint32_t ticks_per_bar = static_cast<std::uint32_t>(kPpq) * 4u;
    // For each bar collect the pitch-class set and assert the expected
    // II-V-I chord quality regardless of seed-selected inversion.
    //   Dm7  (II) pitch-classes: {0, 2, 5, 9}
    //   G7   (V)  pitch-classes: {2, 5, 7, 11}
    //   Cmaj7(I) pitch-classes: {0, 4, 7, 11}
    using PCSet = std::set<std::uint8_t>;
    const PCSet dm7  = {0, 2, 5, 9};
    const PCSet g7   = {2, 5, 7, 11};
    const PCSet cmaj7 = {0, 4, 7, 11};
    for (int bar = 0; bar < 4; ++bar) {
        PCSet pcs;
        const std::uint32_t bar_tick = static_cast<std::uint32_t>(bar) * ticks_per_bar;
        for (const auto& e : events) {
            if (e.tick_on == bar_tick) {
                pcs.insert(static_cast<std::uint8_t>(e.note % 12u));
            }
        }
        ASSERT_FALSE(pcs.empty()) << "bar " << bar << " has no notes";
        switch (bar) {
            case 0: EXPECT_EQ(pcs, dm7);   break; // II (Dm7)
            case 1: EXPECT_EQ(pcs, g7);    break; // V  (G7)
            case 2: EXPECT_EQ(pcs, cmaj7); break; // I  (Cmaj7)
            case 3: EXPECT_EQ(pcs, cmaj7); break; // I  (Cmaj7)
            default: FAIL() << "unexpected bar index";
        }
    }
}

TEST(HarmonyEngineTest, ResolveableViaServiceLocator) {
    auto loc = aimidi::core::make_production();
    auto sp = loc.resolve<IScaleProvider>();
    ASSERT_TRUE(sp);
    EXPECT_TRUE(sp->knows("major"));
    auto he = loc.resolve<IHarmonyEngine>();
    ASSERT_TRUE(he);
    const HarmonyRequest req{.root_key = "C", .scale = "major", .bars = 4, .seed = 42};
    const auto events = he->generate(req);
    EXPECT_FALSE(events.empty());
}
