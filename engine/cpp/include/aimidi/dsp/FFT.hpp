#pragma once
#include <vector>
#include <complex>
#include <cstddef>
#include <cmath>
#include <memory>

namespace aimidi::dsp {

class FFT {
public:
    explicit FFT(std::size_t size);
    ~FFT() = default;

    void forward(const float* input, std::complex<float>* output) const;
    void inverse(const std::complex<float>* input, float* output) const;

    std::size_t size() const { return size_; }
    std::size_t bin_count() const { return size_ / 2 + 1; }

private:
    std::size_t size_;
    std::vector<int> rev_;
    std::vector<std::complex<float>> twiddle_;
};

} // namespace aimidi::dsp
