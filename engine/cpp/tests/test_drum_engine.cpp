#include <aimidi/theory/IDrumEngine.hpp>
#include <aimidi/theory/Types.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <vector>
#include <algorithm>

using namespace aimidi::theory;

namespace {
std::unique_ptr<IDrumEngine> make_engine() {
    return make_drum_engine();
}
}

TEST(DrumEngineTest, DrumEngine_ReturnsNonEmpty) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 9});
    req.seed = 0;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    DrumStyle style{};
    style.seed = 42;
    const auto evs = engine->generate(req, style);
    ASSERT_FALSE(evs.empty());
}

TEST(DrumEngineTest, DrumEngine_SeedDeterministic) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 9});
    req.chords.push_back({static_cast<std::uint32_t>(kPpq) * 4, static_cast<std::uint32_t>(kPpq) * 4, 2, "m7", 9});
    req.seed = 0;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    DrumStyle style{};
    style.seed = 7;
    const auto a = engine->generate(req, style);
    const auto b = engine->generate(req, style);
    EXPECT_EQ(a, b);
}

TEST(DrumEngineTest, DrumEngine_ChannelNineForDrums) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 9});
    req.seed = 0;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    DrumStyle style{};
    style.seed = 99;
    const auto evs = engine->generate(req, style);
    ASSERT_FALSE(evs.empty());
    for (const auto& e : evs) {
        EXPECT_EQ(e.channel, 9u);
    }
}

TEST(DrumEngineTest, DrumEngine_KickSnarePresent) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 9});
    req.seed = 0;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    DrumStyle style{};
    style.seed = 42;
    const auto evs = engine->generate(req, style);
    ASSERT_FALSE(evs.empty());
    bool has_kick = false;
    bool has_snare = false;
    for (const auto& e : evs) {
        if (e.note == 36) has_kick = true;
        if (e.note == 38) has_snare = true;
    }
    EXPECT_TRUE(has_kick);
    EXPECT_TRUE(has_snare);
}

TEST(DrumEngineTest, DrumEngine_GhostNotesExist) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 9});
    req.chords.push_back({static_cast<std::uint32_t>(kPpq) * 4, static_cast<std::uint32_t>(kPpq) * 4, 2, "m7", 9});
    req.seed = 0;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    DrumStyle style{};
    style.seed = 42;
    style.use_ghosts = true;
    const auto evs = engine->generate(req, style);
    ASSERT_FALSE(evs.empty());
    bool has_ghost = false;
    for (const auto& e : evs) {
        if (e.note == 38 && e.velocity < 40) {
            has_ghost = true;
            break;
        }
    }
    EXPECT_TRUE(has_ghost);
}

TEST(DrumEngineTest, DrumEngine_NoNoteAbove127) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 9});
    req.seed = 0;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    DrumStyle style{};
    style.seed = 42;
    const auto evs = engine->generate(req, style);
    ASSERT_FALSE(evs.empty());
    for (const auto& e : evs) {
        EXPECT_LE(e.note, 127u);
        EXPECT_GE(e.note, 0u);
    }
}

TEST(DrumEngineTest, DrumEngine_DurationPositive) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 9});
    req.seed = 0;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    DrumStyle style{};
    style.seed = 42;
    const auto evs = engine->generate(req, style);
    ASSERT_FALSE(evs.empty());
    for (const auto& e : evs) {
        EXPECT_GT(e.tick_off, e.tick_on);
    }
}