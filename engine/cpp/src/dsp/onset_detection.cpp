#include <aimidi/core/Tracy.hpp>
#include <aimidi/dsp/OnsetDetection.hpp>
#include <aimidi/dsp/FFT.hpp>
#include <aimidi/dsp/Windows.hpp>
#include <cmath>
#include <algorithm>
#include <numeric>

namespace aimidi::dsp {

OnsetDetection::OnsetDetection(std::size_t fftSize, std::size_t hopSize)
    : fftSize_(fftSize)
    , hopSize_(hopSize)
    , fft_(std::make_unique<FFT>(fftSize))
    , window_(std::make_unique<Window>(fftSize, WindowType::Hann))
    , prevMag_(fft_->bin_count(), 0.0f)
    , fftBuf_(fftSize, std::complex<float>(0, 0))
    , magBuf_(fft_->bin_count(), 0.0f)
    , inputBuf_(fftSize, 0.0f)
{
}

void OnsetDetection::process(const float* audio, std::size_t numSamples, float /*sampleRate*/) {
    ZoneScoped;
    std::size_t nFrames = (numSamples < fftSize_) ? 0
                        : ((numSamples - fftSize_) / hopSize_) + 1;

    df_.reserve(nFrames);

    for (std::size_t frame = 0; frame < nFrames; ++frame) {
        std::size_t start = frame * hopSize_;

        std::copy(audio + start, audio + start + fftSize_, inputBuf_.begin());

        window_->apply(inputBuf_.data());

        fft_->forward(inputBuf_.data(), fftBuf_.data());

        for (std::size_t b = 0; b < magBuf_.size(); ++b) {
            magBuf_[b] = std::abs(fftBuf_[b]);
        }

        float flux = spectralFlux(magBuf_);

        if (frame >= 1) {
            df_.push_back(flux);
        } else {
            df_.push_back(0.0f);
        }

        std::copy(magBuf_.begin(), magBuf_.end(), prevMag_.begin());
    }
}

float OnsetDetection::spectralFlux(const std::vector<float>& mag) const {
    float sum = 0.0f;
    for (std::size_t b = 0; b < mag.size(); ++b) {
        float diff = mag[b] - prevMag_[b];
        if (diff > 0.0f) {
            sum += diff;
        }
    }
    return sum / static_cast<float>(mag.size());
}

float OnsetDetection::highFrequencyContent(const std::vector<float>& mag) const {
    float sum = 0.0f;
    for (std::size_t b = 0; b < mag.size(); ++b) {
        sum += static_cast<float>(b) * mag[b];
    }
    return sum / static_cast<float>(mag.size());
}

OnsetResult OnsetDetection::getOnsets(float threshold) const {
    OnsetResult result;
    if (df_.empty()) return result;

    float sum = std::accumulate(df_.begin(), df_.end(), 0.0f);
    float mean = sum / static_cast<float>(df_.size());

    float sqSum = 0.0f;
    for (auto v : df_) {
        float d = v - mean;
        sqSum += d * d;
    }
    float stddev = std::sqrt(sqSum / static_cast<float>(df_.size()));

    float thresh = mean + threshold * stddev;

    for (std::size_t i = 1; i + 1 < df_.size(); ++i) {
        if (df_[i] > thresh && df_[i] > df_[i - 1] && df_[i] > df_[i + 1]) {
            result.times.push_back(static_cast<float>(i * hopSize_) / 44100.0f);
            result.strengths.push_back((df_[i] - mean) / stddev);
        }
    }

    return result;
}

} // namespace aimidi::dsp
