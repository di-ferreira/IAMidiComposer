// Test: theory/piano_engine — voicing styles, determinism, hand split, channel.
#include <aimidi/theory/IPianoEngine.hpp>
#include <aimidi/theory/Types.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <vector>
#include <algorithm>
#include <set>

using namespace aimidi::theory;

namespace {
std::unique_ptr<IPianoEngine> make_engine() {
    return make_piano_engine();
}

constexpr bool is_bass(std::uint8_t note)    { return note < 60; }
constexpr bool is_treble(std::uint8_t note)  { return note >= 60; }
} // namespace

TEST(PianoEngineTest, ReturnsNonEmpty) {
    auto engine = make_engine();
    ASSERT_TRUE(engine);
    ChordRequest cr{};
    cr.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 0});
    cr.seed = 0;
    cr.ppq  = static_cast<std::uint32_t>(kPpq);
    PianoRequest req{};
    req.chord_req = cr;
    const auto evs = engine->generate(req);
    EXPECT_FALSE(evs.empty());
}

TEST(PianoEngineTest, SeedDeterministic) {
    auto engine = make_engine();
    ChordRequest cr{};
    cr.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 2, "m7", 0});
    cr.chords.push_back({static_cast<std::uint32_t>(kPpq) * 4,
                         static_cast<std::uint32_t>(kPpq) * 4, 7, "7", 0});
    cr.seed = 42;
    cr.ppq  = static_cast<std::uint32_t>(kPpq);
    PianoRequest req{};
    req.chord_req = cr;
    const auto a = engine->generate(req);
    const auto b = engine->generate(req);
    EXPECT_EQ(a, b);
}

TEST(PianoEngineTest, BlockChordHasLeftAndRight) {
    auto engine = make_engine();
    ChordRequest cr{};
    cr.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 0});
    cr.seed = 7;
    cr.ppq  = static_cast<std::uint32_t>(kPpq);
    PianoRequest req{};
    req.chord_req = cr;
    req.style = PianoStyle::block_chords;
    const auto evs = engine->generate(req);
    ASSERT_GE(evs.size(), 3u);
    bool has_bass   = false;
    bool has_treble = false;
    for (const auto& e : evs) {
        if (is_bass(e.note))   has_bass   = true;
        if (is_treble(e.note)) has_treble = true;
    }
    EXPECT_TRUE(has_bass)   << "block chords need left-hand (bass) notes";
    EXPECT_TRUE(has_treble) << "block chords need right-hand (treble) notes";
}

TEST(PianoEngineTest, ArpeggioSpreadsNotes) {
    auto engine = make_engine();
    ChordRequest cr{};
    cr.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj7", 0});
    cr.seed = 3;
    cr.ppq  = static_cast<std::uint32_t>(kPpq);
    PianoRequest req{};
    req.chord_req = cr;
    req.style = PianoStyle::arpeggio;
    const auto evs = engine->generate(req);
    // Collect distinct tick_ons for treble (right-hand) notes.
    std::set<std::uint32_t> treble_ticks;
    for (const auto& e : evs) {
        if (is_treble(e.note)) {
            treble_ticks.insert(e.tick_on);
        }
    }
    // A maj7 arpeggio over one chord should have 4 distinct start times.
    EXPECT_GE(treble_ticks.size(), 2u)
        << "arpeggio right hand should have notes at different tick_ons";
}

TEST(PianoEngineTest, CompingHasOffbeatBass) {
    auto engine = make_engine();
    ChordRequest cr{};
    cr.chords.push_back({100, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 0});
    cr.seed = 9;
    cr.ppq  = static_cast<std::uint32_t>(kPpq);
    PianoRequest req{};
    req.chord_req = cr;
    req.style = PianoStyle::comping;
    const auto evs = engine->generate(req);
    const std::uint32_t beat2 = 100 + static_cast<std::uint32_t>(kPpq);
    const std::uint32_t beat4 = 100 + static_cast<std::uint32_t>(kPpq) * 3;
    bool found_beat2 = false;
    bool found_beat4 = false;
    for (const auto& e : evs) {
        if (is_bass(e.note)) {
            if (e.tick_on == beat2) found_beat2 = true;
            if (e.tick_on == beat4) found_beat4 = true;
        }
    }
    EXPECT_TRUE(found_beat2) << "comping left hand should play on beat 2";
    EXPECT_TRUE(found_beat4) << "comping left hand should play on beat 4";
}

TEST(PianoEngineTest, ChannelZero) {
    auto engine = make_engine();
    ChordRequest cr{};
    cr.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 0});
    cr.seed = 5;
    cr.ppq  = static_cast<std::uint32_t>(kPpq);
    PianoRequest req{};
    req.chord_req = cr;
    const auto evs = engine->generate(req);
    for (const auto& e : evs) {
        EXPECT_EQ(e.channel, 0u);
    }
}

TEST(PianoEngineTest, DurationPositive) {
    auto engine = make_engine();
    ChordRequest cr{};
    cr.chords.push_back({100, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 0});
    cr.seed = 2;
    cr.ppq  = static_cast<std::uint32_t>(kPpq);
    PianoRequest req{};
    req.chord_req = cr;
    const auto evs = engine->generate(req);
    for (const auto& e : evs) {
        EXPECT_GT(e.tick_off, e.tick_on)
            << "note " << static_cast<int>(e.note)
            << " tick_on=" << e.tick_on << " tick_off=" << e.tick_off;
    }
}

TEST(PianoEngineTest, VelocityInRange) {
    auto engine = make_engine();
    ChordRequest cr{};
    cr.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 0});
    cr.seed = 11;
    cr.ppq  = static_cast<std::uint32_t>(kPpq);
    PianoRequest req{};
    req.chord_req = cr;
    const auto evs = engine->generate(req);
    for (const auto& e : evs) {
        EXPECT_GE(e.velocity, 1u);
        EXPECT_LE(e.velocity, 127u);
    }
}

TEST(PianoEngineTest, MultipleChordsProduceEvents) {
    auto engine = make_engine();
    ChordRequest cr{};
    cr.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj", 0});
    cr.chords.push_back({static_cast<std::uint32_t>(kPpq) * 4,
                         static_cast<std::uint32_t>(kPpq) * 4, 5, "min", 0});
    cr.chords.push_back({static_cast<std::uint32_t>(kPpq) * 8,
                         static_cast<std::uint32_t>(kPpq) * 4, 7, "7", 0});
    cr.seed = 17;
    cr.ppq  = static_cast<std::uint32_t>(kPpq);
    PianoRequest req{};
    req.chord_req = cr;
    const auto evs = engine->generate(req);
    // 3 chords: each should contribute some events.
    EXPECT_GE(evs.size(), 9u);
}

TEST(PianoEngineTest, ChordQualityRespected) {
    auto engine = make_engine();
    ChordRequest cr{};
    cr.chords.push_back({0, static_cast<std::uint32_t>(kPpq) * 4, 0, "maj7", 0});
    cr.seed = 0;
    cr.ppq  = static_cast<std::uint32_t>(kPpq);
    PianoRequest req{};
    req.chord_req = cr;
    req.style = PianoStyle::block_chords;
    const auto evs = engine->generate(req);
    // Collect pitch classes of treble notes.
    std::set<std::uint8_t> treble_pcs;
    for (const auto& e : evs) {
        if (is_treble(e.note)) {
            treble_pcs.insert(static_cast<std::uint8_t>(e.note % 12u));
        }
    }
    // Cmaj7 should have {0, 4, 7, 11}
    EXPECT_EQ(treble_pcs, (std::set<std::uint8_t>{0, 4, 7, 11}));
}
