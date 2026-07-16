#include <aimidi/dsp/ChordDetection.hpp>
#include <aimidi/dsp/FFT.hpp>
#include <aimidi/dsp/Windows.hpp>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <array>

namespace aimidi::dsp {

static constexpr std::array<const char*, 12> kNoteNames = {
    "C", "C#", "D", "D#", "E", "F",
    "F#", "G", "G#", "A", "A#", "B"
};

// Chroma templates for each chord type (root at index 0)
struct ChordTemplate {
    const char* suffix;
    std::array<float, 12> profile;
};

static constexpr std::array<ChordTemplate, 8> kChordTemplates = {{
    { "",    {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f} }, // major
    { "m",   {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f} }, // minor
    { "dim", {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f} }, // diminished
    { "aug", {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f} }, // augmented
    { "sus4",{1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f} }, // sus4
    { "7",   {1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f} }, // dominant 7
    { "maj7",{1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f} }, // major 7
    { "m7",  {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f} }, // minor 7
}};

ChordDetection::~ChordDetection() = default;

ChordDetection::ChordDetection(std::size_t fftSize, std::size_t hopSize, int sampleRate)
    : fftSize_(fftSize)
    , hopSize_(hopSize)
    , sampleRate_(sampleRate)
    , fft_(std::make_unique<FFT>(fftSize))
    , window_(std::make_unique<Window>(fftSize, WindowType::Hann))
{
}

std::vector<ChordResult> ChordDetection::process(const float* audio, std::size_t numSamples) {
    std::vector<ChordResult> results;

    if (numSamples < fftSize_)
        return results;

    std::size_t nFrames = (numSamples - fftSize_) / hopSize_ + 1;

    std::vector<float> inputBuf(fftSize_);
    std::vector<std::complex<float>> fftBuf(fftSize_);
    std::vector<float> magBuf(fft_->bin_count());

    std::size_t analysisInterval = std::max<std::size_t>(1, sampleRate_ / hopSize_ / 4);

    for (std::size_t frame = 0; frame < nFrames; ++frame) {
        std::size_t start = frame * hopSize_;

        std::copy(audio + start, audio + start + fftSize_, inputBuf.begin());

        window_->apply(inputBuf.data());

        fft_->forward(inputBuf.data(), fftBuf.data());

        for (std::size_t b = 0; b < magBuf.size(); ++b) {
            magBuf[b] = std::abs(fftBuf[b]);
        }

        if (frame % analysisInterval == 0) {
            std::array<float, 12> chroma{};
            float binFreq = static_cast<float>(sampleRate_) / static_cast<float>(fftSize_);

            for (std::size_t b = 0; b < magBuf.size(); ++b) {
                float freq = static_cast<float>(b) * binFreq;
                if (freq < 65.0f || freq > 2100.0f)
                    continue;
                float midiNote = 12.0f * std::log2(freq / 440.0f) + 69.0f;
                float pitchClass = std::fmod(midiNote, 12.0f);
                if (pitchClass < 0.0f) pitchClass += 12.0f;
                int binLow = static_cast<int>(pitchClass);
                int binHigh = (binLow + 1) % 12;
                float frac = pitchClass - static_cast<float>(binLow);
                chroma[binLow]  += magBuf[b] * (1.0f - frac);
                chroma[binHigh] += magBuf[b] * frac;
            }

            float sum = std::accumulate(chroma.begin(), chroma.end(), 0.0f);
            if (sum < 1e-4f)
                continue;
            for (auto& v : chroma)
                v /= sum;

            ChordResult cr;
            cr.time_sec = static_cast<float>(frame * hopSize_) / static_cast<float>(sampleRate_);

            float bestCorr = -1.0f;
            int bestRoot = 0;
            int bestType = 0;

            for (int root = 0; root < 12; ++root) {
                for (std::size_t t = 0; t < kChordTemplates.size(); ++t) {
                    float num = 0.0f, denA = 0.0f, denB = 0.0f;
                    for (int i = 0; i < 12; ++i) {
                        int idx = (root + i) % 12;
                        num += chroma[idx] * kChordTemplates[t].profile[i];
                        denA += chroma[idx] * chroma[idx];
                        denB += kChordTemplates[t].profile[i] * kChordTemplates[t].profile[i];
                    }
                    float den = std::sqrt(denA * denB);
                    float corr = (den > 1e-10f) ? num / den : 0.0f;
                    if (corr > bestCorr) {
                        bestCorr = corr;
                        bestRoot = root;
                        bestType = static_cast<int>(t);
                    }
                }
            }

            cr.chord = std::string(kNoteNames[bestRoot]) + kChordTemplates[bestType].suffix;
            cr.confidence = std::max(0.0f, std::min(1.0f, bestCorr));
            results.push_back(cr);
        }
    }

    return results;
}

void ChordDetection::reset() {
    // no persistent state to clear
}

} // namespace aimidi::dsp
