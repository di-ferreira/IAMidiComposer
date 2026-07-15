// Test: theory/rhythm_engine — drum pattern generation, swings, fills.
#include <aimidi/theory/IRhythmEngine.hpp>
#include <aimidi/theory/IChordEngine.hpp>
#include <aimidi/theory/Types.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <vector>
#include <algorithm>
#include <set>

using namespace aimidi::theory;

namespace {
std::unique_ptr<IRhythmEngine> make_engine() {
    return make_rhythm_engine();
}

// Build a simple 4-bar chord request.
ChordRequest four_bar_request(Seed seed) {
    ChordRequest req{};
    req.seed = seed;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    for (int bar = 0; bar < 4; ++bar) {
        req.chords.push_back({
            /*start_tick*/  static_cast<std::uint32_t>(bar) * kPpq * 4,
            /*duration_ticks*/ kPpq * 4,
            /*root_pc*/ 0,
            /*quality*/ "maj",
            /*channel*/ 9
        });
    }
    return req;
}

bool all_on_channel9(const std::vector<MidiEvent>& evs) {
    for (const auto& e : evs) {
        if (e.channel != 9) return false;
    }
    return true;
}

std::vector<std::uint32_t> tick_ons(const std::vector<MidiEvent>& evs) {
    std::vector<std::uint32_t> t;
    t.reserve(evs.size());
    for (const auto& e : evs) t.push_back(e.tick_on);
    std::sort(t.begin(), t.end());
    t.erase(std::unique(t.begin(), t.end()), t.end());
    return t;
}
} // namespace

TEST(RhythmEngineTest, ReturnsNonEmpty) {
    auto engine = make_engine();
    ASSERT_TRUE(engine);
    auto req = four_bar_request(0);
    const RhythmStyle style{};
    const FillSpec fill{};
    const auto evs = engine->generate(req, style, fill);
    EXPECT_FALSE(evs.empty());
}

TEST(RhythmEngineTest, SeedDeterministic) {
    auto engine = make_engine();
    auto req = four_bar_request(42);
    const RhythmStyle style{};
    const FillSpec fill{};
    const auto a = engine->generate(req, style, fill);
    const auto b = engine->generate(req, style, fill);
    EXPECT_EQ(a, b);
}

TEST(RhythmEngineTest, DifferentSeedsDifferent) {
    auto engine = make_engine();
    auto req1 = four_bar_request(1);
    auto req2 = four_bar_request(2);
    const RhythmStyle style{};
    const FillSpec fill{};
    const auto evs1 = engine->generate(req1, style, fill);
    const auto evs2 = engine->generate(req2, style, fill);
    // Different seeds may produce different velocities or patterns.
    // At minimum the vectors should differ in at least one event.
    bool any_different = false;
    const std::size_t max_cmp = std::min(evs1.size(), evs2.size());
    for (std::size_t i = 0; i < max_cmp; ++i) {
        if (evs1[i].tick_on != evs2[i].tick_on ||
            evs1[i].note != evs2[i].note ||
            evs1[i].velocity != evs2[i].velocity) {
            any_different = true;
            break;
        }
    }
    EXPECT_TRUE(any_different || evs1.size() != evs2.size());
}

TEST(RhythmEngineTest, SwingModifiesTiming) {
    auto engine = make_engine();

    ChordRequest req{};
    req.seed = 7;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    req.chords.push_back({
        0u, kPpq * 4, 0, "maj", 9
    });

    RhythmStyle straight{};
    straight.swing = 0.0f;
    straight.resolution = RhythmResolution::eighth;
    straight.seed = 0;

    RhythmStyle swung{};
    swung.swing = 0.5f;
    swung.resolution = RhythmResolution::eighth;
    swung.seed = 0;

    const FillSpec fill{};

    const auto ev_straight = engine->generate(req, straight, fill);
    const auto ev_swung   = engine->generate(req, swung, fill);

    const auto ticks_straight = tick_ons(ev_straight);
    const auto ticks_swung    = tick_ons(ev_swung);

    // Swung should have different tick positions than straight.
    EXPECT_NE(ticks_straight, ticks_swung);
}

TEST(RhythmEngineTest, SixteenthFinerGrid) {
    auto engine = make_engine();
    auto req = four_bar_request(0);

    RhythmStyle eighth{};
    eighth.resolution = RhythmResolution::eighth;
    eighth.seed = 0;

    RhythmStyle sixteenth{};
    sixteenth.resolution = RhythmResolution::sixteenth;
    sixteenth.seed = 0;

    const FillSpec fill{};

    const auto ev_eighth = engine->generate(req, eighth, fill);
    const auto ev_sixteenth = engine->generate(req, sixteenth, fill);

    // Sixteenth should produce more events than eighth.
    EXPECT_GT(ev_sixteenth.size(), ev_eighth.size());
}

TEST(RhythmEngineTest, FillAtBar4) {
    auto engine = make_engine();
    auto req = four_bar_request(0);

    RhythmStyle style{};
    style.resolution = RhythmResolution::eighth;
    style.seed = 0;

    FillSpec fill_no;
    fill_no.start_bar = -1;
    fill_no.strength = 1;

    FillSpec fill_at_3;
    fill_at_3.start_bar = 3;  // bar index 3 = 4th bar
    fill_at_3.strength = 2;

    const auto ev_no_fill = engine->generate(req, style, fill_no);
    const auto ev_fill    = engine->generate(req, style, fill_at_3);

    // Fill should add extra notes (open HH) compared to no fill.
    EXPECT_GE(ev_fill.size(), ev_no_fill.size());

    // Verify open HH (note 46) appears in fill output.
    bool has_open_hh = false;
    for (const auto& e : ev_fill) {
        if (e.note == 46) { has_open_hh = true; break; }
    }
    EXPECT_TRUE(has_open_hh);
}

TEST(RhythmEngineTest, Channel9ForDrums) {
    auto engine = make_engine();
    auto req = four_bar_request(0);
    const RhythmStyle style{};
    const FillSpec fill{};
    const auto evs = engine->generate(req, style, fill);
    ASSERT_FALSE(evs.empty());
    EXPECT_TRUE(all_on_channel9(evs));
}