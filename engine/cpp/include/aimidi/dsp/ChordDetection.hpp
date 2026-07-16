#pragma once
#include <vector>
#include <array>
#include <string>
#include <cstddef>
#include <memory>

namespace aimidi::dsp {

class FFT;
class Window;

struct ChordResult {
    std::string chord;      // e.g. "C", "G7", "Am"
    float       confidence;
    float       time_sec;
};

class ChordDetection {
public:
    ChordDetection(std::size_t fftSize, std::size_t hopSize, int sampleRate);
    ~ChordDetection();

    // Process audio and return detected chords over time.
    std::vector<ChordResult> process(const float* audio, std::size_t numSamples);

    // Reset state.
    void reset();

private:
    std::string classifyChord(const std::array<float, 12>& chroma) const;

    std::size_t fftSize_;
    std::size_t hopSize_;
    int sampleRate_;
    std::unique_ptr<FFT> fft_;
    std::unique_ptr<Window> window_;
};

} // namespace aimidi::dsp
