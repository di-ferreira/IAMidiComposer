// Test: theory/scales.
#include <aimidi/theory/IScaleProvider.hpp>
#include <memory>
#include <gtest/gtest.h>

using namespace aimidi::theory;

namespace {
std::unique_ptr<IScaleProvider> make_provider() {
    return make_scale_provider();
}
} // namespace

TEST(ScaleProviderTest, KnowsMajorAndMinor) {
    auto sp = make_provider();
    ASSERT_TRUE(sp);
    EXPECT_TRUE(sp->knows("major"));
    EXPECT_TRUE(sp->knows("minor"));
    EXPECT_TRUE(sp->knows("blues"));
    EXPECT_FALSE(sp->knows("does_not_exist"));
}

TEST(ScaleProviderTest, MajorScaleIntervals) {
    auto sp = make_provider();
    const auto iv = sp->intervals_of("major");
    ASSERT_EQ(iv.size(), 7u);
    EXPECT_EQ(iv[0], 0);
    EXPECT_EQ(iv[1], 2);
    EXPECT_EQ(iv[2], 4);
    EXPECT_EQ(iv[3], 5);
    EXPECT_EQ(iv[4], 7);
    EXPECT_EQ(iv[5], 9);
    EXPECT_EQ(iv[6], 11);
}

TEST(ScaleProviderTest, MinorScaleIntervals) {
    auto sp = make_provider();
    const auto iv = sp->intervals_of("minor");
    ASSERT_EQ(iv.size(), 7u);
    EXPECT_EQ(iv[2], 3);   // minor third
    EXPECT_EQ(iv[5], 8);  // minor sixth
    EXPECT_EQ(iv[6], 10); // minor seventh
}

TEST(ScaleProviderTest, UnknownReturnsEmpty) {
    auto sp = make_provider();
    const auto iv = sp->intervals_of("totally_made_up");
    EXPECT_TRUE(iv.empty());
}

TEST(ScaleProviderTest, PentatonicHasFiveNotes) {
    auto sp = make_provider();
    EXPECT_EQ(sp->intervals_of("pentatonic_major").size(), 5u);
    EXPECT_EQ(sp->intervals_of("pentatonic_minor").size(), 5u);
}

// ── ScaleProvider v2: church modes (diatonic rotations of major scale) ──
TEST(ScaleProviderTest, IonianModeIntervals) {
    auto sp = make_provider();
    const auto iv = sp->intervals_of("ionian");
    ASSERT_EQ(iv.size(), 7u);
    EXPECT_EQ(iv[0], 0); EXPECT_EQ(iv[1], 2); EXPECT_EQ(iv[2], 4);
    EXPECT_EQ(iv[3], 5); EXPECT_EQ(iv[4], 7); EXPECT_EQ(iv[5], 9);
    EXPECT_EQ(iv[6], 11);
}

TEST(ScaleProviderTest, DorianModeIntervals) {
    auto sp = make_provider();
    const auto iv = sp->intervals_of("dorian");
    ASSERT_EQ(iv.size(), 7u);
    EXPECT_EQ(iv[0], 0); EXPECT_EQ(iv[1], 2); EXPECT_EQ(iv[2], 3);
    EXPECT_EQ(iv[3], 5); EXPECT_EQ(iv[4], 7); EXPECT_EQ(iv[5], 9);
    EXPECT_EQ(iv[6], 10);
}

TEST(ScaleProviderTest, PhrygianModeIntervals) {
    auto sp = make_provider();
    const auto iv = sp->intervals_of("phrygian");
    ASSERT_EQ(iv.size(), 7u);
    EXPECT_EQ(iv[0], 0); EXPECT_EQ(iv[1], 1); EXPECT_EQ(iv[2], 3);
    EXPECT_EQ(iv[3], 5); EXPECT_EQ(iv[4], 7); EXPECT_EQ(iv[5], 8);
    EXPECT_EQ(iv[6], 10);
}

TEST(ScaleProviderTest, LydianModeIntervals) {
    auto sp = make_provider();
    const auto iv = sp->intervals_of("lydian");
    ASSERT_EQ(iv.size(), 7u);
    EXPECT_EQ(iv[0], 0); EXPECT_EQ(iv[1], 2); EXPECT_EQ(iv[2], 4);
    EXPECT_EQ(iv[3], 6); EXPECT_EQ(iv[4], 7); EXPECT_EQ(iv[5], 9);
    EXPECT_EQ(iv[6], 11);
}

TEST(ScaleProviderTest, MixolydianModeIntervals) {
    auto sp = make_provider();
    const auto iv = sp->intervals_of("mixolydian");
    ASSERT_EQ(iv.size(), 7u);
    EXPECT_EQ(iv[0], 0); EXPECT_EQ(iv[1], 2); EXPECT_EQ(iv[2], 4);
    EXPECT_EQ(iv[3], 5); EXPECT_EQ(iv[4], 7); EXPECT_EQ(iv[5], 9);
    EXPECT_EQ(iv[6], 10);
}

TEST(ScaleProviderTest, AeolianModeIntervals) {
    auto sp = make_provider();
    const auto iv = sp->intervals_of("aeolian");
    ASSERT_EQ(iv.size(), 7u);
    EXPECT_EQ(iv[0], 0); EXPECT_EQ(iv[1], 2); EXPECT_EQ(iv[2], 3);
    EXPECT_EQ(iv[3], 5); EXPECT_EQ(iv[4], 7); EXPECT_EQ(iv[5], 8);
    EXPECT_EQ(iv[6], 10);
}

TEST(ScaleProviderTest, LocrianModeIntervals) {
    auto sp = make_provider();
    const auto iv = sp->intervals_of("locrian");
    ASSERT_EQ(iv.size(), 7u);
    EXPECT_EQ(iv[0], 0); EXPECT_EQ(iv[1], 1); EXPECT_EQ(iv[2], 3);
    EXPECT_EQ(iv[3], 5); EXPECT_EQ(iv[4], 6); EXPECT_EQ(iv[5], 8);
    EXPECT_EQ(iv[6], 10);
}

// ── intervals_for: transposition ──
TEST(ScaleProviderTest, TransposedMajorScaleG) {
    auto sp = make_provider();
    const auto iv = sp->intervals_for("major", 7);
    ASSERT_EQ(iv.size(), 7u);
    // G major normalized mod 12: {7,9,11,0,2,4,6}.
    EXPECT_EQ(iv[0], 7);
    EXPECT_EQ(iv[1], 9);
    EXPECT_EQ(iv[2], 11);
    EXPECT_EQ(iv[3], 0);
    EXPECT_EQ(iv[4], 2);
    EXPECT_EQ(iv[5], 4);
    EXPECT_EQ(iv[6], 6);
}

TEST(ScaleProviderTest, TransposedUnknownReturnsEmpty) {
    auto sp = make_provider();
    const auto iv = sp->intervals_for("not_a_scale", 0);
    EXPECT_TRUE(iv.empty());
}
