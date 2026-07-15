#include <aimidi/theory/IStyleDetection.hpp>
#include <gtest/gtest.h>
#include <memory>

using namespace aimidi::theory;

TEST(StyleDetectionTest, ReturnsNonEmpty) {
    auto detector = make_style_detection();
    ASSERT_TRUE(detector);
    auto info = detector->analyze_text("pop song", 0);
    EXPECT_FALSE(info.genre.empty());
}

TEST(StyleDetectionTest, DetectsRock) {
    auto detector = make_style_detection();
    auto info = detector->analyze_text("rock guitar riff", 0);
    EXPECT_EQ(info.genre, "rock");
}

TEST(StyleDetectionTest, DetectsJazz) {
    auto detector = make_style_detection();
    auto info = detector->analyze_text("jazz piano trio", 0);
    EXPECT_EQ(info.genre, "jazz");
}

TEST(StyleDetectionTest, DetectsElectronic) {
    auto detector = make_style_detection();
    auto info = detector->analyze_text("electronic dance track", 0);
    EXPECT_EQ(info.genre, "electronic");
}

TEST(StyleDetectionTest, AudioAnalysisReturnsDefault) {
    auto detector = make_style_detection();
    std::vector<float> features = {0.1f, 0.2f, 0.3f};
    auto info = detector->analyze_audio(features, 0);
    EXPECT_EQ(info.genre, "pop");
    EXPECT_FLOAT_EQ(info.confidence, 0.3f);
}

TEST(StyleDetectionTest, SeedDeterministic) {
    auto detector = make_style_detection();
    auto a = detector->analyze_text("test", 42);
    auto b = detector->analyze_text("test", 42);
    EXPECT_EQ(a.genre, b.genre);
    EXPECT_EQ(a.mood, b.mood);
}
