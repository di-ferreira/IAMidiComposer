// Test: theory/chord_engine — quality parsing, durations, determinism.
//
// Chord voicings apply a seed-deterministic inversion; tests therefore assert
// the PITCH-CLASS SET (invariant under inversion) and durations rather than
// exact MIDI pitches.
#include <aimidi/theory/IChordEngine.hpp>
#include <aimidi/theory/Types.hpp>

#include <gtest/gtest.h>

#include <memory>
#include <vector>
#include <algorithm>
#include <set>

using namespace aimidi::theory;

namespace {
std::unique_ptr<IChordEngine> make_engine() {
    return make_chord_engine();
}

// Collect sorted, deduplicated pitch classes (mod 12).
std::set<std::uint8_t> pcs_of(const std::vector<MidiEvent>& evs) {
    std::set<std::uint8_t> pcs;
    for (const auto& e : evs) {
        pcs.insert(static_cast<std::uint8_t>(e.note % 12u));
    }
    return pcs;
}

// Collect sorted absolute pitches.
std::vector<std::uint8_t> sorted_pitches(const std::vector<MidiEvent>& evs) {
    std::vector<std::uint8_t> notes;
    notes.reserve(evs.size());
    for (const auto& e : evs) {
        notes.push_back(e.note);
    }
    std::sort(notes.begin(), notes.end());
    return notes;
}
} // namespace

TEST(ChordEngineTest, MajorTriadHasThreeNotes) {
    auto engine = make_engine();
    ASSERT_TRUE(engine);
    ChordRequest req{};
    req.chords.push_back({ /*start_tick*/0, /*dur*/static_cast<std::uint32_t>(kPpq)*4,
                           /*root_pc*/0, /*quality*/"maj", /*ch*/0 });
    req.seed = 0;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    const auto evs = engine->voicing(req);
    ASSERT_EQ(evs.size(), 3u);
    // C major triad pitch-classes: {0, 4, 7}.
    const auto pcs = pcs_of(evs);
    EXPECT_EQ(pcs, (std::set<std::uint8_t>{0, 4, 7}));
    // Lowest pitch is one of the chord tones within an octave of 48.
    const auto notes = sorted_pitches(evs);
    EXPECT_GE(notes.front(), 36u);
    EXPECT_LE(notes.back(), 67u);
}

TEST(ChordEngineTest, Dominant7HasFourNotes) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({ 0, static_cast<std::uint32_t>(kPpq)*4, 0, "7", 0 });
    req.seed = 1;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    const auto evs = engine->voicing(req);
    ASSERT_EQ(evs.size(), 4u);
    // C7 pitch-classes: {0, 4, 7, 10}.
    const auto pcs = pcs_of(evs);
    EXPECT_EQ(pcs, (std::set<std::uint8_t>{0, 4, 7, 10}));
}

TEST(ChordEngineTest, Minor7HasFourNotes) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({ 0, static_cast<std::uint32_t>(kPpq)*4, 0, "m7", 0 });
    req.seed = 2;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    const auto evs = engine->voicing(req);
    ASSERT_EQ(evs.size(), 4u);
    // Cm7 pitch-classes: {0, 3, 7, 10}.
    const auto pcs = pcs_of(evs);
    EXPECT_EQ(pcs, (std::set<std::uint8_t>{0, 3, 7, 10}));
}

TEST(ChordEngineTest, HalfDiminished) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({ 0, static_cast<std::uint32_t>(kPpq)*4, 0, "m7b5", 0 });
    req.seed = 3;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    const auto evs = engine->voicing(req);
    ASSERT_EQ(evs.size(), 4u);
    // Cm7b5 pitch-classes: {0, 3, 6, 10}.
    const auto pcs = pcs_of(evs);
    EXPECT_EQ(pcs, (std::set<std::uint8_t>{0, 3, 6, 10}));
}

TEST(ChordEngineTest, DurationIsRespected) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({ /*start_tick*/1000u, /*dur*/2000u, 0, "maj", 0 });
    req.seed = 7;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    const auto evs = engine->voicing(req);
    ASSERT_FALSE(evs.empty());
    for (const auto& e : evs) {
        EXPECT_EQ(e.tick_on, 1000u);
        EXPECT_EQ(e.tick_off, 1000u + 2000u);
    }
}

TEST(ChordEngineTest, SeedDeterministic) {
    auto engine = make_engine();
    ChordRequest req{};
    req.chords.push_back({ 0, static_cast<std::uint32_t>(kPpq)*4, 2, "m7", 0 });
    req.chords.push_back({ static_cast<std::uint32_t>(kPpq)*4, static_cast<std::uint32_t>(kPpq)*4, 7, "7", 0 });
    req.chords.push_back({ static_cast<std::uint32_t>(kPpq)*8, static_cast<std::uint32_t>(kPpq)*4, 0, "maj7", 0 });
    req.seed = 42;
    req.ppq  = static_cast<std::uint32_t>(kPpq);
    const auto a = engine->voicing(req);
    const auto b = engine->voicing(req);
    EXPECT_EQ(a, b);
}
