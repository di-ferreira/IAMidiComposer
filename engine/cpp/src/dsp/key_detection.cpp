#include <aimidi/dsp/KeyDetection.hpp>
#include <aimidi/dsp/FFT.hpp>
#include <aimidi/dsp/Windows.hpp>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <array>
#include <string>

namespace aimidi::dsp {

// Krumhansl-Schmuckler key profiles
// Index 0 = C, 1 = C#, ... 11 = B
static constexpr std::array<float, 12> kMajorProfile = {
    6.35f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f,
    2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f
};

static constexpr std::array<float, 12> kMinorProfile = {
    6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f,
    2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f
};

static constexpr std::array<const char*, 12> kKeyNames = {
    "C", "C#", "D", "D#", "E", "F",
    "F#", "G", "G#", "A", "A#", "B"
};

KeyDetection::~KeyDetection() = default;

KeyDetection::KeyDetection(std::size_t fftSize, std::size_t hopSize, int sampleRate)
    : fftSize_(fftSize)
    , hopSize_(hopSize)
    , sampleRate_(sampleRate)
    , fft_(std::make_unique<FFT>(fftSize))
    , window_(std::make_unique<Window>(fftSize, WindowType::Hann))
{
}

KeyResult KeyDetection::process(const float* audio, std::size_t numSamples) {
    if (numSamples < fftSize_)
        return estimateKey();

    std::size_t nFrames = (numSamples - fftSize_) / hopSize_ + 1;

    std::vector<float> inputBuf(fftSize_);
    std::vector<std::complex<float>> fftBuf(fftSize_);
    std::vector<float> magBuf(fft_->bin_count());

    for (std::size_t frame = 0; frame < nFrames; ++frame) {
        std::size_t start = frame * hopSize_;

        std::copy(audio + start, audio + start + fftSize_, inputBuf.begin());

        window_->apply(inputBuf.data());

        fft_->forward(inputBuf.data(), fftBuf.data());

        for (std::size_t b = 0; b < magBuf.size(); ++b) {
            magBuf[b] = std::abs(fftBuf[b]);
        }

        updateChroma(magBuf, static_cast<float>(sampleRate_));
        ++frameCount_;
    }

    return estimateKey();
}

void KeyDetection::updateChroma(const std::vector<float>& mag, float sampleRate) {
    float binFreq = sampleRate / static_cast<float>(fftSize_);

    for (std::size_t b = 0; b < mag.size(); ++b) {
        float freq = static_cast<float>(b) * binFreq;
        if (freq < 65.0f || freq > 2100.0f)
            continue;
        float midiNote = 12.0f * std::log2(freq / 440.0f) + 69.0f;
        float pitchClass = std::fmod(midiNote, 12.0f);
        if (pitchClass < 0.0f) pitchClass += 12.0f;
        int binLow = static_cast<int>(pitchClass);
        int binHigh = (binLow + 1) % 12;
        float frac = pitchClass - static_cast<float>(binLow);
        chroma_[binLow]  += mag[b] * (1.0f - frac);
        chroma_[binHigh] += mag[b] * frac;
    }
}

KeyResult KeyDetection::estimateKey() const {
    KeyResult result;
    result.chroma = chroma_;

    if (frameCount_ == 0) {
        result.key = "C";
        result.scale = "major";
        result.confidence = 0.0f;
        return result;
    }

    // Normalize chroma
    float sum = std::accumulate(chroma_.begin(), chroma_.end(), 0.0f);
    std::array<float, 12> normChroma{};
    if (sum > 1e-10f) {
        for (int i = 0; i < 12; ++i)
            normChroma[i] = chroma_[i] / sum;
    }

    // Correlate with all 24 key profiles
    float bestCorr = -1.0f;
    int bestKey = 0;
    bool bestIsMajor = true;

    for (int root = 0; root < 12; ++root) {
        auto correlate = [&](const std::array<float, 12>& profile) -> float {
            float meanX = 0.0f, meanY = 0.0f;
            for (int i = 0; i < 12; ++i) {
                meanX += normChroma[(root + i) % 12];
                meanY += profile[i];
            }
            meanX /= 12.0f;
            meanY /= 12.0f;
            float num = 0.0f, denA = 0.0f, denB = 0.0f;
            for (int i = 0; i < 12; ++i) {
                int idx = (root + i) % 12;
                float dx = normChroma[idx] - meanX;
                float dy = profile[i] - meanY;
                num += dx * dy;
                denA += dx * dx;
                denB += dy * dy;
            }
            float den = std::sqrt(denA * denB);
            return (den > 1e-10f) ? num / den : 0.0f;
        };

        float corrMajor = correlate(kMajorProfile);
        if (corrMajor > bestCorr) {
            bestCorr = corrMajor;
            bestKey = root;
            bestIsMajor = true;
        }

        float corrMinor = correlate(kMinorProfile);
        if (corrMinor > bestCorr) {
            bestCorr = corrMinor;
            bestKey = root;
            bestIsMajor = false;
        }
    }

    result.key = kKeyNames[bestKey];
    result.scale = bestIsMajor ? "major" : "minor";
    result.confidence = std::max(0.0f, std::min(1.0f, bestCorr));

    return result;
}

void KeyDetection::reset() {
    chroma_.fill(0.0f);
    frameCount_ = 0;
}

} // namespace aimidi::dsp
