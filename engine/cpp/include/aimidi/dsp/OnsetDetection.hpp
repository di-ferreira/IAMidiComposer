#pragma once
#include <vector>
#include <complex>
#include <cstddef>
#include <memory>

namespace aimidi::dsp {

class FFT;
class Window;

struct OnsetResult {
    std::vector<float> times;
    std::vector<float> strengths;
};

class OnsetDetection {
public:
    OnsetDetection(std::size_t fftSize, std::size_t hopSize);
    ~OnsetDetection() = default;

    void process(const float* audio, std::size_t numSamples, float sampleRate);

    OnsetResult getOnsets(float threshold = 1.5) const;

    const std::vector<float>& detectionFunction() const { return df_; }

private:
    float spectralFlux(const std::vector<float>& mag) const;
    float highFrequencyContent(const std::vector<float>& mag) const;

    std::size_t fftSize_;
    std::size_t hopSize_;
    std::unique_ptr<FFT> fft_;
    std::unique_ptr<Window> window_;

    std::vector<float> df_;
    std::vector<float> prevMag_;
    std::vector<std::complex<float>> fftBuf_;
    std::vector<float> magBuf_;
    std::vector<float> inputBuf_;
    std::size_t inputPos_ = 0;
};

} // namespace aimidi::dsp
