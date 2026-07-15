// Test: theory/bass_engine — patterns, determinism, range, channel.
#include <aimidi/theory/IBassEngine.hpp>
#include <aimidi/theory/Types.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <vector>
#include <algorithm>

using namespace aimidi::theory;

namespace {
std::unique_ptr<IBassEngine> make_engine() {
    return make_bass_engine();
}
} // namespace

TEST(BassEngineTest, ReturnsNonEmpty) {
    auto engine = make_engine();
    ASSERT_TRUE(engine);
    ChordRequest req{};
    req.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 0});
    req.seed = 0;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    BassStyle style{};
    const auto evs = engine->generate(req, style);
    EXPECT_FALSE(evs.empty());
}

TEST(BassEngineTest, SeedDeterministic) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 2, "m7", 0});
    req.chords.push_back({static_cast<std::uint32_t>(kPpq) * 4,
                          static_cast<std::uint32_t>(kPpq) * 4, 7, "7", 0});
    req.seed = 42;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    BassStyle style{};
    const auto a = engine->generate(req, style);
    const auto b = engine->generate(req, style);
    EXPECT_EQ(a, b);
}

TEST(BassEngineTest, RespectsChordRoot) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 0});
    req.seed = 7;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    BassStyle style{};
    const auto evs = engine->generate(req, style);
    ASSERT_FALSE(evs.empty());
    // All notes should be in the C major chord (C=0, E=4, G=7).
    for (const auto& e : evs) {
        EXPECT_TRUE(e.note % 12 == 0 || e.note % 12 == 4 || e.note % 12 == 7)
            << "note " << static_cast<int>(e.note) << " is not in C major chord";
    }
}

TEST(BassEngineTest, SameSeedSamePattern) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 0});
    req.seed = 123;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    BassStyle style{};
    const auto a = engine->generate(req, style);
    const auto b = engine->generate(req, style);
    EXPECT_EQ(a.size(), b.size());
    for (std::size_t i = 0; i < a.size(); ++i) {
        EXPECT_EQ(a[i].tick_on, b[i].tick_on);
        EXPECT_EQ(a[i].tick_off, b[i].tick_off);
        EXPECT_EQ(a[i].note, b[i].note);
    }
}

TEST(BassEngineTest, VelocityInRange) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 0});
    req.seed = 9;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    BassStyle style{};
    const auto evs = engine->generate(req, style);
    for (const auto& e : evs) {
        EXPECT_GE(e.velocity, 40u);
        EXPECT_LE(e.velocity, 127u);
    }
}

TEST(BassEngineTest, DurationPositive) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({100, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 0});
    req.seed = 5;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    BassStyle style{};
    const auto evs = engine->generate(req, style);
    for (const auto& e : evs) {
        EXPECT_GT(e.tick_off, e.tick_on);
    }
}

TEST(BassEngineTest, ChannelZero) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 0});
    req.seed = 3;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    BassStyle style{};
    const auto evs = engine->generate(req, style);
    for (const auto& e : evs) {
        EXPECT_EQ(e.channel, 0u);
    }
}