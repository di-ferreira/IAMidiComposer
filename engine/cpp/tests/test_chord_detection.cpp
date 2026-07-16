#include <aimidi/dsp/ChordDetection.hpp>
#include <gtest/gtest.h>
#include <cmath>

using namespace aimidi::dsp;

TEST(ChordDetectionTest, CreatesEngine) {
    ChordDetection cd(4096, 2048, 44100);
    SUCCEED();
}

TEST(ChordDetectionTest, CMajorChord) {
    // Use C5 (523.25), E5 (659.25), G5 (783.99)
    ChordDetection cd(4096, 2048, 44100);
    std::vector<float> audio(44100, 0.0f);
    for (std::size_t i = 0; i < audio.size(); ++i) {
        float sample = 0.0f;
        sample += std::sin(2.0 * M_PI * 523.25 * i / 44100.0);
        sample += std::sin(2.0 * M_PI * 659.25 * i / 44100.0);
        sample += std::sin(2.0 * M_PI * 783.99 * i / 44100.0);
        audio[i] = sample * 0.3f;
    }
    auto chords = cd.process(audio.data(), audio.size());
    EXPECT_FALSE(chords.empty());
    EXPECT_EQ(chords[0].chord.substr(0, 1), "C");
}

TEST(ChordDetectionTest, AminorChord) {
    // Use A4 (440), C5 (523.25), E5 (659.25)
    ChordDetection cd(4096, 2048, 44100);
    std::vector<float> audio(44100, 0.0f);
    for (std::size_t i = 0; i < audio.size(); ++i) {
        float sample = 0.0f;
        sample += std::sin(2.0 * M_PI * 440.0 * i / 44100.0);
        sample += std::sin(2.0 * M_PI * 523.25 * i / 44100.0);
        sample += std::sin(2.0 * M_PI * 659.25 * i / 44100.0);
        audio[i] = sample * 0.3f;
    }
    auto chords = cd.process(audio.data(), audio.size());
    EXPECT_FALSE(chords.empty());
    EXPECT_EQ(chords[0].chord.substr(0, 1), "A");
}

TEST(ChordDetectionTest, G7Chord) {
    // Use G4 (392), B4 (493.88), D5 (587.33), F5 (698.46)
    ChordDetection cd(4096, 2048, 44100);
    std::vector<float> audio(44100, 0.0f);
    for (std::size_t i = 0; i < audio.size(); ++i) {
        float sample = 0.0f;
        sample += std::sin(2.0 * M_PI * 392.0 * i / 44100.0);
        sample += std::sin(2.0 * M_PI * 493.88 * i / 44100.0);
        sample += std::sin(2.0 * M_PI * 587.33 * i / 44100.0);
        sample += std::sin(2.0 * M_PI * 698.46 * i / 44100.0);
        audio[i] = sample * 0.3f;
    }
    auto chords = cd.process(audio.data(), audio.size());
    EXPECT_FALSE(chords.empty());
}

TEST(ChordDetectionTest, SilenceReturnsEmpty) {
    ChordDetection cd(4096, 2048, 44100);
    std::vector<float> audio(4096, 0.0f);
    auto chords = cd.process(audio.data(), audio.size());
    EXPECT_TRUE(chords.empty());
}
