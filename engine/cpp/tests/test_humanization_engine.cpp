// Test: theory/humanization_engine — micro-timing, velocity, swing, grid snap.
//
// All assertions verify seed-deterministic behaviour and invariant preservation.
#include <aimidi/theory/IHumanizationEngine.hpp>
#include <aimidi/theory/Types.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <vector>
#include <algorithm>
#include <cstdint>

using namespace aimidi::theory;

namespace {

std::unique_ptr<IHumanizationEngine> make_engine() {
    return make_humanization_engine();
}

// Build a simple event sequence for testing.
std::vector<MidiEvent> make_test_events() {
    std::vector<MidiEvent> out;
    out.reserve(4);
    out.push_back({   0u, 120u, 0, 60, 100, 0u });
    out.push_back({ 120u, 240u, 0, 64, 100, 0u });
    out.push_back({ 240u, 360u, 0, 67, 100, 0u });
    out.push_back({ 360u, 480u, 0, 72, 100, 0u });
    return out;
}

} // namespace

TEST(HumanizationEngine_DoesNotCrashOnEmpty, EmptyInput) {
    auto engine = make_engine();
    ASSERT_TRUE(engine);
    std::vector<MidiEvent> empty;
    HumanizationParams params{};
    params.seed = 0;
    EXPECT_NO_THROW(engine->apply(empty, params));
}

TEST(HumanizationEngine_SeedDeterministic, SameSeedSameOutput) {
    auto engine = make_engine();
    auto evs_a = make_test_events();
    auto evs_b = make_test_events();

    HumanizationParams params{};
    params.seed = 42;
    params.timing_jitter_ticks = 5;
    params.velocity_variation = 10;

    engine->apply(evs_a, params);
    engine->apply(evs_b, params);

    EXPECT_EQ(evs_a, evs_b);
}

TEST(HumanizationEngine_TimingJitterChangesTickOn, JitterShiftsTickOn) {
    auto engine = make_engine();
    auto evs_original = make_test_events();
    auto evs_jittered = make_test_events();

    HumanizationParams params{};
    params.seed = 123;
    params.timing_jitter_ticks = 5;
    params.velocity_variation = 0;   // isolate timing jitter

    engine->apply(evs_jittered, params);

    bool any_changed = false;
    for (std::size_t i = 0; i < evs_original.size(); ++i) {
        if (evs_jittered[i].tick_on != evs_original[i].tick_on) {
            any_changed = true;
            break;
        }
    }
    EXPECT_TRUE(any_changed);
}

TEST(HumanizationEngine_VelocityVariationChangesVelocity, VariationChangesVelocity) {
    auto engine = make_engine();
    auto evs_original = make_test_events();
    auto evs_varied   = make_test_events();

    HumanizationParams params{};
    params.seed = 777;
    params.timing_jitter_ticks = 0;  // isolate velocity variation
    params.velocity_variation = 10;

    engine->apply(evs_varied, params);

    bool any_changed = false;
    for (std::size_t i = 0; i < evs_original.size(); ++i) {
        if (evs_varied[i].velocity != evs_original[i].velocity) {
            any_changed = true;
            break;
        }
    }
    EXPECT_TRUE(any_changed);
}

TEST(HumanizationEngine_VelocityClamped, VelocityInRange1To127) {
    auto engine = make_engine();
    // Use events at extreme velocities to test clamping.
    std::vector<MidiEvent> events;
    events.push_back({0u, 120u, 0, 60,   0, 0u});
    events.push_back({0u, 120u, 0, 64, 127, 0u});

    HumanizationParams params{};
    params.seed = 42;
    params.timing_jitter_ticks = 0;
    params.velocity_variation = 200;   // huge range forces clamping

    engine->apply(events, params);

    for (const auto& ev : events) {
        EXPECT_GE(ev.velocity, 1u);
        EXPECT_LE(ev.velocity, 127u);
    }
}

TEST(HumanizationEngine_GrooveSwingDelaysOffbeats, SwingDelaysOffBeatEighthNotes) {
    auto engine = make_engine();
    // Events on off-beat eighth-note positions (tick_on % 480 == 240).
    std::vector<MidiEvent> events;
    events.push_back({ 240u, 360u, 0, 60, 100, 0u });
    events.push_back({ 720u, 840u, 0, 64, 100, 0u });
    events.push_back({1200u,1320u, 0, 67, 100, 0u });

    auto original = events;

    HumanizationParams params{};
    params.seed = 0;
    params.timing_jitter_ticks = 0;
    params.velocity_variation = 0;
    params.groove_swing = 0.5f;

    engine->apply(events, params);

    for (std::size_t i = 0; i < events.size(); ++i) {
        EXPECT_GT(events[i].tick_on, original[i].tick_on)
            << "Event " << i << " at tick " << original[i].tick_on << " should be delayed";
    }
}

TEST(HumanizationEngine_SnapToGridQuantizes, SnapRoundsToNearestGrid) {
    auto engine = make_engine();
    std::vector<MidiEvent> events;
    events.push_back({  10u, 130u, 0, 60, 100, 0u });
    events.push_back({ 200u, 350u, 0, 64, 100, 0u });

    HumanizationParams params{};
    params.seed = 0;
    params.timing_jitter_ticks = 0;
    params.velocity_variation = 0;
    params.snap_to_grid = true;
    params.grid_resolution = 120;

    engine->apply(events, params);

    for (const auto& ev : events) {
        EXPECT_EQ(ev.tick_on % 120u, 0u);
        EXPECT_EQ(ev.tick_off % 120u, 0u);
    }
}

TEST(HumanizationEngine_TickOffGreaterThanTickOn, InvariantPreserved) {
    auto engine = make_engine();
    // Events with durations short enough that jitter could collapse them.
    std::vector<MidiEvent> events;
    events.push_back({ 100u, 101u, 0, 60, 100, 0u });  // duration = 1 tick
    events.push_back({ 200u, 201u, 0, 64, 100, 0u });

    HumanizationParams params{};
    params.seed = 99;
    params.timing_jitter_ticks = 10;  // larger than some durations
    params.velocity_variation = 10;
    params.groove_swing = 1.0f;       // also shifts ticks

    engine->apply(events, params);

    for (const auto& ev : events) {
        EXPECT_GT(ev.tick_off, ev.tick_on);
    }
}

TEST(HumanizationEngine_TickOnNonNegative, ClampedToZero) {
    auto engine = make_engine();
    std::vector<MidiEvent> events;
    events.push_back({ 0u, 120u, 0, 60, 100, 0u });

    HumanizationParams params{};
    params.seed = 1;
    params.timing_jitter_ticks = 10;  // could shift below 0
    params.velocity_variation = 0;

    engine->apply(events, params);

    EXPECT_GE(events[0].tick_on, 0u);
    EXPECT_GT(events[0].tick_off, events[0].tick_on);
}
