#pragma once
#include <vector>
#include <array>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

namespace aimidi::dsp {

class FFT;
class Window;

struct KeyResult {
    std::string key;              // e.g. "C", "G#", "F"
    std::string scale;            // "major" or "minor"
    float       confidence;       // 0.0 to 1.0
    std::array<float, 12> chroma; // the 12-bin chromagram
};

class KeyDetection {
public:
    KeyDetection(std::size_t fftSize, std::size_t hopSize, int sampleRate);
    ~KeyDetection();

    // Process audio and return detected key.
    KeyResult process(const float* audio, std::size_t numSamples);

    // Get the accumulated chromagram.
    const std::array<float, 12>& chromagram() const { return chroma_; }

    // Reset state.
    void reset();

private:
    void updateChroma(const std::vector<float>& mag, float sampleRate);
    KeyResult estimateKey() const;

    std::size_t fftSize_;
    std::size_t hopSize_;
    int sampleRate_;
    std::unique_ptr<FFT> fft_;
    std::unique_ptr<Window> window_;
    std::array<float, 12> chroma_{};
    std::size_t frameCount_ = 0;
};

} // namespace aimidi::dsp
