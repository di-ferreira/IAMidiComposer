#include <aimidi/dsp/FFT.hpp>
#include <aimidi/dsp/Windows.hpp>
#include <aimidi/dsp/OnsetDetection.hpp>
#include <gtest/gtest.h>
#include <cmath>

using namespace aimidi::dsp;

TEST(FFTTest, ForwardInverse) {
    FFT fft(1024);
    std::vector<float> input(1024, 0.0f);
    input[0] = 1.0f;
    std::vector<std::complex<float>> spectrum(1024);
    std::vector<float> output(1024);
    fft.forward(input.data(), spectrum.data());
    fft.inverse(spectrum.data(), output.data());
    EXPECT_NEAR(output[0], 1.0f, 1e-5);
}

TEST(FFTTest, SineWave) {
    FFT fft(1024);
    std::vector<float> input(1024);
    for (std::size_t i = 0; i < 1024; ++i)
        input[i] = std::sin(2.0 * M_PI * 50.0 * i / 44100.0);
    std::vector<std::complex<float>> spectrum(1024);
    fft.forward(input.data(), spectrum.data());
    float max_mag = 0;
    std::size_t max_bin = 0;
    for (std::size_t i = 0; i < fft.bin_count(); ++i) {
        float mag = std::abs(spectrum[i]);
        if (mag > max_mag) { max_mag = mag; max_bin = i; }
    }
    EXPECT_EQ(max_bin, 1);
}

TEST(FFTTest, MultipleSizes) {
    for (auto size : {256, 512, 1024, 2048}) {
        FFT fft(size);
        std::vector<float> input(size, 0.0f);
        input[0] = 1.0f;
        std::vector<std::complex<float>> spectrum(size);
        std::vector<float> output(size);
        fft.forward(input.data(), spectrum.data());
        fft.inverse(spectrum.data(), output.data());
        EXPECT_NEAR(output[0], 1.0f, 1e-5);
    }
}

TEST(WindowTest, HannSum) {
    Window w(1024, WindowType::Hann);
    double sum = 0;
    for (std::size_t i = 0; i < w.size(); ++i)
        sum += w.coefficients()[i];
    EXPECT_NEAR(sum, 512.0, 10.0);
}

TEST(WindowTest, HammingSum) {
    Window w(1024, WindowType::Hamming);
    double sum = 0;
    for (std::size_t i = 0; i < w.size(); ++i)
        sum += w.coefficients()[i];
    EXPECT_NEAR(sum, 552.0, 20.0);
}

TEST(WindowTest, AllTypes) {
    for (auto type : {WindowType::Hann, WindowType::Hamming, WindowType::Blackman, WindowType::BlackmanHarris, WindowType::Kaiser}) {
        Window w(256, type);
        EXPECT_EQ(w.size(), 256);
        for (std::size_t i = 0; i < w.size(); ++i)
            EXPECT_GE(w.coefficients()[i], -1e-6f);
    }
}

TEST(OnsetDetectionTest, ImpulseOnset) {
    OnsetDetection od(1024, 512);
    // Generate audio with a clear onset: silence then sine tone
    std::vector<float> audio(4096, 0.0f);
    // First half: silence
    for (std::size_t i = 0; i < 2048; ++i)
        audio[i] = 0.0f;
    // Second half: 440 Hz sine tone
    for (std::size_t i = 2048; i < 4096; ++i)
        audio[i] = std::sin(2.0 * M_PI * 440.0 * i / 44100.0);
    od.process(audio.data(), audio.size(), 44100.0f);
    auto result = od.getOnsets(1.0f);
    EXPECT_FALSE(result.times.empty());
}

TEST(OnsetDetectionTest, SilenceNoOnset) {
    OnsetDetection od(1024, 512);
    std::vector<float> audio(2048, 0.0f);
    od.process(audio.data(), audio.size(), 44100.0f);
    auto result = od.getOnsets(1.5f);
    EXPECT_TRUE(result.times.empty());
}

TEST(OnsetDetectionTest, MultipleFrames) {
    OnsetDetection od(1024, 512);
    std::vector<float> audio(4096, 0.0f);
    od.process(audio.data(), audio.size(), 44100.0f);
    auto result = od.getOnsets(1.5f);
    EXPECT_TRUE(result.times.empty());
}