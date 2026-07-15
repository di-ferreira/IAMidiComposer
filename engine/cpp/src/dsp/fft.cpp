#include <aimidi/dsp/FFT.hpp>
#include <cassert>
#include <numeric>

namespace aimidi::dsp {

FFT::FFT(std::size_t size) : size_(size) {
    assert((size & (size - 1)) == 0);

    rev_.resize(size);
    for (std::size_t i = 0; i < size; ++i) {
        rev_[i] = i;
        std::size_t j = 0;
        for (std::size_t bit = 1; bit < size; bit <<= 1) {
            j = (j << 1) | ((i & bit) ? 1 : 0);
        }
        rev_[i] = j;
    }

    twiddle_.resize(size / 2);
    for (std::size_t i = 0; i < size / 2; ++i) {
        double angle = -2.0 * M_PI * i / size;
        twiddle_[i] = std::complex<float>(std::cos(angle), std::sin(angle));
    }
}

void FFT::forward(const float* input, std::complex<float>* output) const {
    for (std::size_t i = 0; i < size_; ++i)
        output[i] = std::complex<float>(input[rev_[i]], 0.0f);

    for (std::size_t len = 2; len <= size_; len <<= 1) {
        std::size_t half = len / 2;
        for (std::size_t i = 0; i < size_; i += len) {
            for (std::size_t j = 0; j < half; ++j) {
                auto w = twiddle_[j * size_ / len];
                auto u = output[i + j];
                auto v = output[i + j + half] * w;
                output[i + j] = u + v;
                output[i + j + half] = u - v;
            }
        }
    }
}

void FFT::inverse(const std::complex<float>* input, float* output) const {
    auto buf = std::make_unique<std::complex<float>[]>(size_);
    for (std::size_t i = 0; i < size_; ++i)
        buf[i] = std::conj(input[rev_[i]]);

    for (std::size_t len = 2; len <= size_; len <<= 1) {
        std::size_t half = len / 2;
        for (std::size_t i = 0; i < size_; i += len) {
            for (std::size_t j = 0; j < half; ++j) {
                auto w = twiddle_[j * size_ / len];
                auto u = buf[i + j];
                auto v = buf[i + j + half] * w;
                buf[i + j] = u + v;
                buf[i + j + half] = u - v;
            }
        }
    }

    float inv = 1.0f / size_;
    for (std::size_t i = 0; i < size_; ++i)
        output[i] = std::conj(buf[i]).real() * inv;
}

} // namespace aimidi::dsp
