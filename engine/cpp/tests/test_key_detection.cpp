#include <aimidi/dsp/KeyDetection.hpp>
#include <gtest/gtest.h>
#include <cmath>

using namespace aimidi::dsp;

TEST(KeyDetectionTest, CreatesEngine) {
    KeyDetection kd(4096, 2048, 44100);
    SUCCEED();
}

TEST(KeyDetectionTest, CMajorDetected) {
    // Use C5 (523.25), E5 (659.25), G5 (783.99) for better FFT resolution
    KeyDetection kd(4096, 2048, 44100);
    std::vector<float> audio(88200, 0.0f);  // 2 seconds
    for (std::size_t i = 0; i < audio.size(); ++i) {
        float sample = 0.0f;
        sample += std::sin(2.0 * M_PI * 523.25 * i / 44100.0);  // C5
        sample += std::sin(2.0 * M_PI * 659.25 * i / 44100.0);  // E5
        sample += std::sin(2.0 * M_PI * 783.99 * i / 44100.0);  // G5
        audio[i] = sample * 0.3f;
    }
    auto result = kd.process(audio.data(), audio.size());
    EXPECT_EQ(result.key, "C");
    EXPECT_GT(result.confidence, 0.3f);
}

TEST(KeyDetectionTest, AminorDetected) {
    // Use A4 (440), C5 (523.25), E5 (659.25)
    KeyDetection kd(4096, 2048, 44100);
    std::vector<float> audio(88200, 0.0f);
    for (std::size_t i = 0; i < audio.size(); ++i) {
        float sample = 0.0f;
        sample += std::sin(2.0 * M_PI * 440.0 * i / 44100.0);   // A4
        sample += std::sin(2.0 * M_PI * 523.25 * i / 44100.0);  // C5
        sample += std::sin(2.0 * M_PI * 659.25 * i / 44100.0);  // E5
        audio[i] = sample * 0.3f;
    }
    auto result = kd.process(audio.data(), audio.size());
    EXPECT_EQ(result.key, "A");
    EXPECT_GT(result.confidence, 0.3f);
}

TEST(KeyDetectionTest, ChromaNotEmpty) {
    KeyDetection kd(4096, 2048, 44100);
    std::vector<float> audio(44100, 0.0f);
    for (std::size_t i = 0; i < audio.size(); ++i)
        audio[i] = std::sin(2.0 * M_PI * 440.0 * i / 44100.0);
    auto result = kd.process(audio.data(), audio.size());
    float sum = 0;
    for (auto v : result.chroma) sum += v;
    EXPECT_GT(sum, 0.0f);
}

TEST(KeyDetectionTest, ResetClears) {
    KeyDetection kd(4096, 2048, 44100);
    std::vector<float> audio(44100, 0.0f);
    for (std::size_t i = 0; i < audio.size(); ++i)
        audio[i] = std::sin(2.0 * M_PI * 440.0 * i / 44100.0);
    kd.process(audio.data(), audio.size());
    kd.reset();
    float sum = 0;
    for (auto v : kd.chromagram()) sum += v;
    EXPECT_EQ(sum, 0.0f);
}
